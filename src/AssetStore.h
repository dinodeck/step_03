#ifndef ASSETSTORE_H
#define ASSETSTORE_H
#include <string>
#include <map>
#include "Asset.h"

class IAssetLoader;
struct lua_State;

class AssetStore : IAssetLoader
{
private:
	std::map<std::string, Asset> mStore;
    Asset* mManifest;
    bool ReloadAssets();
    struct AssetDef
    {
        const char* name;
        const char* path;
        AssetDef(const char* name, const char* path) :
            name(name), path(path) {}
    };
    bool LoadLuaTableToAssetDefs(lua_State* state,
                                 const char* tableName,
                                 std::map<std::string, AssetStore::AssetDef>& destination);
    bool LoadAssetSubTable(lua_State* state, const char* tableName, const char* path);
    bool LoadAssetDef(lua_State* state,
                      std::map<std::string, AssetStore::AssetDef>& destination);
public:
	AssetStore();
	~AssetStore();

    static bool IsOutOfDate(Asset& asset);
    static time_t GetModifiedTimeStamp(Asset& asset);

    // Callbacks for Assets
    virtual bool OnAssetReload(Asset& asset);
    virtual void OnAssetDestroyed(Asset& asset);

	void Add(const char* name, const char* path, IAssetLoader* callback);

    // Check the manifest
	bool Reload();
    bool Reload(const std::string& manifestPath);
    // Load in the asset file, if it's not changed.
    //bool LoadFromFile(const char* path);
    bool AssetExists(const char* name);
    Asset* GetAssetByName(const char* name);
    void Clear();
};

#endif