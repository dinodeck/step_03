#ifndef ASSETSTORE_H
#define ASSETSTORE_H
#include <string>
#include <map>
#include "Asset.h"

class IAssetOwner;
struct lua_State;

class AssetStore : IAssetOwner
{
private:

	std::map<std::string, Asset> mStore;
    std::map<std::string, IAssetOwner*> mAssetOwnerMap;

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
    void RemoveAssetsNotInManifest();
public:
	AssetStore();
	~AssetStore();

    static bool IsOutOfDate(Asset& asset);
    static time_t GetModifiedTimeStamp(Asset& asset);

    // Callbacks for Assets
    virtual bool OnAssetReload(Asset& asset);
    virtual void OnAssetDestroyed(Asset& asset);

	void    Add(const char* name, const char* path, IAssetOwner* callback);
	bool    Reload();
    bool    Reload(const std::string& manifestPath);
    bool    AssetExists(const char* name);
    Asset*  GetAssetByName(const char* name);
    void    Clear();

    // Used when loading assets from the manifest
    void RegisterAssetOwner(const char* name, IAssetOwner* callback);
};

#endif