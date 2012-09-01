#ifndef ASSETSTORE_H
#define ASSETSTORE_H
#include <string>
#include <map>
#include "Asset.h"

class IAssetLoader;

class AssetStore : IAssetLoader
{
private:
	std::map<std::string, Asset> mStore;

public:
	AssetStore();
	~AssetStore();

    static bool IsOutOfDate(Asset& asset);
    static time_t GetModifiedTimeStamp(Asset& asset);

    // Callbacks for Assets
    virtual bool OnAssetReload(Asset& asset);
    virtual void OnAssetDestroyed(Asset& asset);

	void Add(const char* name, const char* path, IAssetLoader* callback);

    // Go through all assets and try and reload
	void Reload();
    // Load in the asset file, if it's not changed.
    //bool LoadFromFile(const char* path);
    bool AssetExists(const char* name);
    Asset* GetAssetByName(const char* name);
    void Clear();
};

#endif