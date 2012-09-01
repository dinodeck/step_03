#include "AssetStore.h"
#include "IAssetLoader.h"
#include <map>
#include <list>
#include <assert.h>
#include <string>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "Asset.h"
#include "LuaState.h"



extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

AssetStore::AssetStore()
{
	mManifest = NULL;
}

AssetStore::~AssetStore()
{
	if(mManifest)
	{
		delete mManifest;
	}
}

void AssetStore::Add(const char* name, const char* path, IAssetLoader* callback)
{
	mStore.insert(std::pair<std::string, Asset>(
					std::string(name),
					Asset(name, path, callback)));
}

bool AssetStore::AssetExists(const char* name)
{
	return mStore.find(name) != mStore.end();
}

Asset* AssetStore::GetAssetByName(const char* name)
{
	std::map<std::string, Asset>::iterator iter = mStore.find(name);
	if(iter == mStore.end())
	{
		return NULL;
	}
	return &(iter->second);
}

// bool AssetStore::LoadFromFile(const char* path)
// {
// 	//
// 	// This already getting complicated!
// 	// Needs simplifying!
// 	//
// 	printf("Load from file called: %s\n", path);

// 	if(FileExists(path)) // 0. Does this path exist?
// 	{
// 		//0.Yes
// 		//1. Do we already have the Asset file?
// 		if(AssetExists(AssetFileId()))
// 		{
// 			Asset* AssetFile = GetAssetByName(AssetFileId());

// 			// 2. Has the path been changed
// 			// 2.Yes -
// 			//	Clear previous data
// 			//  Update the path
// 			//  Set the Asset as not loaded.
// 			ClearAssetsFromFile();


// 			// 2.No
// 			// Fine everything else should get taken care of
// 		}
// 		else
// 		{
// 			Add(AssetFileId(), path, this);
// 		}
// 	}
// 	else
// 	{
// 		// 0.No issue a warning
// 		printf("Failed to load resource file: [%s]\n", path);
// 		printf("Resource file is specified in settings.lua e.g. manifest=\"manifest.lua\"\n");
// 		// It may exist in the future, so add it
// 		if(ResourceExists(ResourceFileId()))
// 		{
// 			// Need to clear loaded resources
// 			ClearResourcesFromFile();
// 			mStore.erase(mStore.find(ResourceFileId()));
// 		}

// 		// and add it
// 		Add(ResourceFileId(), path, this);
// 		// because the file doesn't exist at this time
// 		// don't try and load it.
// 		return false;
// 	}

// 	return true;
// }


void AssetStore::Clear()
{
	if(mManifest)
	{
		delete mManifest;
		mManifest = NULL;
	}
	for(std::map<std::string, Asset>::iterator it = mStore.begin(); it != mStore.end(); ++it)
	{
		Asset& asset = it->second;
		asset.OnDestroy();
		mStore.erase( it++ );
	}
}



bool AssetStore::OnAssetReload(Asset& asset)
{
	// Script files and the asset itself?
	return false;
}

void AssetStore::OnAssetDestroyed(Asset& asset)
{
}

bool AssetStore::LoadAssetDef(lua_State* state, std::map<std::string, AssetDef>& destination)
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

	printf("Successfully called LoadResource [%s]\n", name.c_str());
	return true;
}

bool AssetStore::LoadLuaTableToAssetDefs(lua_State* state, const char* tableName, std::map<std::string, AssetDef>& destination)
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
		 	lua_pop(state, 1);	// remove value, keep key for next iter
		}
	}
	lua_pop(state, 1);
	return true;
}

bool AssetStore::LoadAssetSubTable(lua_State* state, const char* tableName, const char* path)
{
	std::map<std::string, AssetStore::AssetDef> scriptAssets;
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
	for(std::map<std::string, AssetStore::AssetDef>::iterator
	    iter = scriptAssets.begin();
	    iter != scriptAssets.end();
	    ++iter)
	{
		// If the key doesn't exist in the asset store, we should add it.
		bool shouldAdd = false;

		AssetStore::AssetDef& assetDef = iter->second;

		std::map<std::string, Asset>::iterator storeIter = mStore.find(iter->first);
		if(storeIter == mStore.end())
		{
			shouldAdd = true;
		}
		else
		{
			// It already exists
			Asset& asset = storeIter->second;

			// a. Check the path
			if(asset.Path() != assetDef.path)
			{
				asset.OnDestroy();
				mStore.erase( storeIter );
				shouldAdd = true;
			}
			else
			{
				// It's added, the name is the same, mark it as touched.
				asset.Touch(true);
			}
		}


		if(shouldAdd)
		{

			// The 'this' here depends on the script type
			// Probably GAME for scripts
			// TEXTURE MANAGER for everything else
			Asset asset(assetDef.name, assetDef.path, this);
			asset.Touch(true);
			mStore.insert(std::pair<std::string, Asset>(
			              assetDef.name,
			              asset
			              ));
		}

	}
	return true;
}

bool AssetStore::Reload(const std::string& manifestPath)
{
	const char* path = manifestPath.c_str();

	if(mManifest &&
	   mManifest->Path() == manifestPath &&
	   !AssetStore::IsOutOfDate(*mManifest))
	{
		// Easier nothing has changed in the manifest, just reload the children.
		printf("[%s] SKIPPED.\n", manifestPath.c_str());
		return ReloadAssets();
	}
	else
	{
		// Deleting and recreating the manifest asset keeps the Asset Class
		// simple. Even through I don't really messing with memory outside of
		// constructors / destructors
		if(mManifest)
		{
			delete mManifest;
			mManifest = NULL;
		}
		mManifest = new Asset("manifest", path, this);
		mManifest->SetTimeLastModified(GetModifiedTimeStamp(*mManifest));

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

			for(std::map<std::string, Asset>::iterator
			    iter = mStore.begin();
			    iter != mStore.end();
			    ++iter)
			{
				// Reset touch flag
				iter->second.Touch(false);
			}

			bool success = LoadAssetSubTable(state, "scripts", path);
			if(!success)
			{
				lua_close(state);
				return false;
			}


			std::list< std::map<std::string, Asset>::iterator > iteratorList;

			// Gather up elements that need to be deleted
			for(std::map<std::string, Asset>::iterator
			    iter = mStore.begin();
			    iter != mStore.end();
			    ++iter)
			{
				// Are there any lua-asset that weren't touched? Remove them.
				if(!iter->second.IsTouched())
				{
					printf("Removing [%s] as it's no longer in the manifest.\n",
					iter->second.Path().c_str());
					iter->second.OnDestroy();
					iteratorList.push_back(iter);
				}
			}

			// Actually delete the elements.
			for(std::list< std::map<std::string, Asset>::iterator >::iterator
			    iter = iteratorList.begin();
			    iter != iteratorList.end();
			    ++iter)
			{

			    mStore.erase(*iter);
			}


		}
		mManifest->SetIsLoaded(true);
		printf("Here, after loading the script resources.\n");
		return true;
	}
}

bool AssetStore::Reload()
{
	assert(mManifest);
	return Reload(mManifest->Path());
}


bool AssetStore::ReloadAssets()
{
	// Iterate through files,
	// Load if not loaded
	// Else get last modified date
	// If date is later than store date then reload
	for(std::map<std::string, Asset>::iterator it = mStore.begin(); it != mStore.end(); ++it)
	{
		Asset& asset = it->second;

		// Be careful when loading from a package.
		struct stat s;
		time_t lastModified = time(NULL);
		if(stat(asset.Path().c_str(), &s) == 0)
		{
			lastModified = s.st_mtime;
		}

		if(asset.IsLoaded() && lastModified <= asset.LastModified())
		{
			printf("%s already loaded.\n", asset.Name().c_str());
		}
		else
		{
			std::string timeString(ctime(&lastModified));
			// Remove trailing \n from time string
			{
				size_t end = timeString.find_last_of('\n');
				timeString = timeString.substr(0, end);
			}
			printf("Loading [%s] Last Modified [%s]\n",
			       asset.Name().c_str(),
			       timeString.c_str());
			bool success = asset.OnReload();
			if(!success)
			{
				asset.SetIsLoaded(false);
				printf("Fail to load [%s]\n", asset.Name().c_str());
				return false;
			}
			asset.SetTimeLastModified(lastModified);
		}
	}
	return true;
}

time_t AssetStore::GetModifiedTimeStamp(Asset& asset)
{
	struct stat s;
	if(stat(asset.Path().c_str(), &s) == 0)
	{
		return s.st_mtime;
	}
	return -1;
}

bool AssetStore::IsOutOfDate(Asset& asset)
{
	if(!asset.IsLoaded())
	{
		return true;
	}

	time_t lastModified = AssetStore::GetModifiedTimeStamp(asset);
	return lastModified > asset.LastModified();
}