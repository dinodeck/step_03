#include "ManifestAssetStore.h"

#include <assert.h>
#include <list>
#include <map>
#include <string>

#include "Asset.h"
#include "DancingSquidLua.h"
#include "LuaState.h"

ManifestAssetStore::ManifestAssetStore()
    : mManifest(NULL), mAssetStore()
{
    printf("ManifestAssetStore Constructed.\n");
}

ManifestAssetStore::~ManifestAssetStore()
{
    if(mManifest)
    {
        delete mManifest;
        mManifest = NULL;
    }
}

void ManifestAssetStore::Clear()
{
    if(mManifest)
    {
        delete mManifest;
        mManifest = NULL;
    }
    mAssetStore.Clear();
}

//
// From a manifest in a LuaState load asset definitions into a map
//
bool ManifestAssetStore::LoadAssetDef(lua_State* state, std::map<std::string, AssetDef>& destination)
{
    assert(state);
    // Error check - is the id a string?
    if(!lua_isstring(state, -2))
    {
        printf("ERROR: Asset id expected got [%s].\n",
        lua_typename(state, lua_type(state, -2)));
        return false;
    }
    std::string name = std::string(lua_tostring(state, -2));

    lua_pushstring(state, "path");
    lua_rawget(state, -2);

    if(lua_istable(state, -2))
    {
        if(!lua_isstring(state,  -1))
        {
            printf("ERROR: Ignoring [%s] because path type invalid. Expected type string got type [%s]\n.",
                   name.c_str(),
                   lua_typename(state, lua_type(state, -1)));

            lua_pop(state, 1);
            return false;
        }

        std::string path = std::string(lua_tostring(state, -1));
        lua_pop(state, 1);
        destination.insert(std::pair<std::string, AssetDef>
                    (
                        name,
                        AssetDef(name.c_str(), path.c_str()))
                    );
    }
    else
    {
        lua_pop(state, 1);
        printf("Error: [%s] has no path.\n", name.c_str());
        return false;
    }

    printf("Successfully called LoadAssetDef [%s]\n", name.c_str());
    return true;
}


bool ManifestAssetStore::LoadLuaTableToAssetDefs(lua_State* state,
    const char* tableName,
    std::map<std::string, AssetDef>& destination)
{
    lua_pushstring(state, tableName); // -1 in stack
    lua_rawget(state, -2);
    if(lua_istable(state, -1))
    {
        printf("Parsing %s.\n", tableName);
        // Iterate through k, v and create resource
        // Entries
        lua_pushnil(state);  // first key
        while (lua_next(state, -2) != 0)
        {
            bool success = true;
            success = LoadAssetDef(state, destination);
            if(!success)
            {
                return false;
            }
            lua_pop(state, 1);  // remove value, keep key for next iter
        }
    }
    lua_pop(state, 1);
    return true;
}

// Takes tables like this
// scripts =
// {
//     ['main.lua'] =
//     {
//          path = "/test/main.lua"
//     }
//     ... etc
// }
// And updates the in memory asset store intelligently.
bool ManifestAssetStore::LoadAssetSubTable(lua_State* state, const char* tableName, const char* path)
{

    // ASSERT THAT THE ASSET-TABLE HAS A HANDLER FOR THE ASSETS
    assert(mAssetOwnerMap.find( std::string(tableName) ) != mAssetOwnerMap.end());
    IAssetOwner* assetOwner = mAssetOwnerMap.find( std::string(tableName) )->second;

    std::map<std::string, ManifestAssetStore::AssetDef> scriptAssets;
    bool isLoaded = false;
    isLoaded = LoadLuaTableToAssetDefs(state, "scripts", scriptAssets);
    if(!isLoaded)
    {
        printf("Failed to parse [%s].\n", path);
        printf("[%s]\n", lua_tostring(state, -1));
        return false;
    }

    // Go through - has the path changed? Unload, update path
    // Are there any additional resources add them
    for(std::map<std::string, ManifestAssetStore::AssetDef>::iterator
        iter = scriptAssets.begin();
        iter != scriptAssets.end();
        ++iter)
    {
        ManifestAssetStore::AssetDef& assetDef = iter->second;
        const char* assetName = iter->first.c_str();

        Asset* asset = mAssetStore.GetAssetByName(assetName);
        if(!asset)
        {
            // If the key doesn't exist in the asset store, we should add it.
            Asset* asset = mAssetStore.Add(assetDef.name, assetDef.path, assetOwner);
            asset->Touch(true);
            continue;
        }

        // It already exists
        // a. Check the path
        if(asset->Path() != assetDef.path)
        {
            mAssetStore.Remove(assetName);
            Asset* newAsset = mAssetStore.Add(assetDef.name, assetDef.path, assetOwner);
            newAsset->Touch(true);
        }
        else
        {
            // It's added, the name is the same, mark it as touched.
            asset->Touch(true);
        }
    }
    return true;
}



bool ManifestAssetStore::Reload()
{
    assert(mManifest);
    return Reload(mManifest->Path());
}

bool ManifestAssetStore::Reload(const std::string& manifestPath)
{
    const char* path = manifestPath.c_str();

    if(mManifest &&
       mManifest->Path() == manifestPath &&
       !AssetStore::IsOutOfDate(*mManifest))
    {
        // Easier nothing has changed in the manifest, just reload the children.
        printf("[%s] SKIPPED.\n", manifestPath.c_str());
        return mAssetStore.Reload();
    }
    else
    {
        printf("Here currently dont have a manifest.\n");
        // Deleting and recreating the manifest asset keeps the Asset Class
        // simple. Even through I don't really messing with memory outside of
        // constructors / destructors
        if(mManifest)
        {
            delete mManifest;
            mManifest = NULL;
        }
        mManifest = new Asset("manifest", path, this);
        mManifest->SetTimeLastModified
        (
            AssetStore::GetModifiedTimeStamp(*mManifest)
        );

        // OK the manifest has changed, this effects ALL the assets
        // some might no longer be refernced and need removing
        // Some might be new and need adding, so first thing is to
        // read in the manifest
        LuaState luaState("Manifest");
        bool success = luaState.DoFile(path);
        if(!success)
        {
            // Should just do a clear?
            // But this way don't unload everything if there's a minor mistake
            // in the manifest file.
            delete mManifest;
            mManifest = NULL;
            return false;
        }



        // Load in the manifest lua file
        // Manifest is expected to have global table 'manifest'.
        lua_State* state = luaState.State();
        lua_getglobal(state, "manifest");
        if(!lua_istable(state, -1))
        {
            printf("Error: [manifest] table missing in [%s]. Expected:\n",  path);
            printf("manifest =\n");
            printf("{\n");
            printf("    scripts =\n");
            printf("    {\n");
            printf("        ...\n");
            printf("    },\n");
            printf("}\n");
            lua_close(state);
            return false;
        }

        {
            //
            // Modify the store of assets
            // 1. Removing ones that the manifest on longer specifies
            // 2. Add ones that are brand new
            // 3. Telling ones to be reloading if the path has changed.
            //

            mAssetStore.ResetTouchFlag();

            bool success = LoadAssetSubTable(state, "scripts", path);
            if(!success)
            {
                lua_close(state);
                return false;
            }

            RemoveAssetsNotInManifest();
            mAssetStore.Reload();
        }
        mManifest->SetIsLoaded(true);
        printf("Here, after loading the script resources.\n");
        return true;
    }
}

// Assets the have a touch flag equaling false are removed.
void ManifestAssetStore::RemoveAssetsNotInManifest()
{
    mAssetStore.RemoveUntouchedAssets();
}

bool ManifestAssetStore::OnAssetReload(Asset& asset)
{
    // Script files and the asset itself?
    return false;
}

void ManifestAssetStore::OnAssetDestroyed(Asset& asset)
{
}

// Used when loading assets from the manifest
void ManifestAssetStore::RegisterAssetOwner(const char* name, IAssetOwner* callback)
{
    mAssetOwnerMap[std::string(name)] = callback;
}