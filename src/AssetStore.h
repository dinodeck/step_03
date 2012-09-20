#ifndef ASSETSTORE_H
#define ASSETSTORE_H
#include <string>
#include <map>
#include "Asset.h"

class IAssetOwner;
struct lua_State;

class AssetStore
{
private:
    static bool CleverReloading;
	std::map<std::string, Asset> mStore;

public:
    static void CleverReloadingFlag(bool value)
    {
        AssetStore::CleverReloading = value;
    }
	AssetStore();
	~AssetStore();

    static bool IsOutOfDate(Asset& asset);
    static time_t GetModifiedTimeStamp(Asset& asset);

    Asset*  Add(const char* name, const char* path, IAssetOwner* callback);
    Asset*  GetAssetByName(const char* name);
    bool    AssetExists(const char* name);
    bool    Reload();
    void    Clear();
    void    Remove(const char* name);
    void    RemoveUntouchedAssets();
    void    ResetTouchFlag();
};

#endif