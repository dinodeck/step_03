#ifndef MANIFESTASSETSTORE_H
#define MANIFESTASSETSTORE_H

#include <map>
#include <string>

#include "IAssetOwner.h"
#include "AssetStore.h"


class Asset;
struct lua_State;

//
// An asset store controlled by an manifest file.
//
class ManifestAssetStore : IAssetOwner
{
private:
    Asset* mManifest;
    AssetStore mAssetStore;

    // Who handles what by default
    // [.lua] -> DancingSquid etc
    std::map<std::string, IAssetOwner*> mAssetOwnerMap;

    struct AssetDef
    {
        const char* name;
        const char* path;
        AssetDef(const char* name, const char* path) :
            name(name), path(path) {}
    };
    bool LoadLuaTableToAssetDefs(lua_State* state,
                                 const char* tableName,
                                 std::map<std::string, ManifestAssetStore::AssetDef>& destination);
    bool LoadAssetSubTable(lua_State* state, const char* tableName, const char* path);
    bool LoadAssetDef(lua_State* state,
                      std::map<std::string, ManifestAssetStore::AssetDef>& destination);
    void RemoveAssetsNotInManifest();
public:
    ManifestAssetStore();
    ~ManifestAssetStore();

    // Callbacks for Assets
    virtual bool OnAssetReload(Asset& asset);
    virtual void OnAssetDestroyed(Asset& asset);

    bool AssetExists(const char* name)
    {
        return mAssetStore.AssetExists(name);
    }

    Asset* GetAssetByName(const char* name)
    {
        return mAssetStore.GetAssetByName(name);
    }

    bool Reload(const std::string& manifest);
    bool Reload();
    void Clear();

    // Used when loading assets from the manifest
    void RegisterAssetOwner(const char* name, IAssetOwner* callback);

};

#endif