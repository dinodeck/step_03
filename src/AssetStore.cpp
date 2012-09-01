#include "AssetStore.h"
#include "IAssetLoader.h"
#include <map>
#include <string>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "Asset.h"

AssetStore::AssetStore()
{

}

AssetStore::~AssetStore()
{

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

// void ResourceStore::ClearResourcesFromFile()
// {
// 	for(std::map<std::string, Resource>::iterator it = mStore.begin(); it != mStore.end(); ++it)
// 	{
// 		Resource& resource = it->second;
// 		if(IsLoadedByResourceFile(resource))
// 		{
// 			mStore.erase( it++ );
// 		}
// 	}
// }

void AssetStore::Clear()
{
	for(std::map<std::string, Asset>::iterator it = mStore.begin(); it != mStore.end(); ++it)
	{
		Asset& asset = it->second;
		asset.OnDestroy();
		mStore.erase( it++ );
	}
}

// This was a bad idea!
// bool AssetStore::IsLoadedByAssetFile(const Resource& resource)
// {
// 	if(asset.Name() == AssetFileId()
// 	   || asset.Name() == "settings")
// 	{
// 		return false;
// 	}
// 	return true;
// }

bool AssetStore::OnAssetReload(Asset& asset)
{
	// Script files and the asset itself?
	return false;
}

void AssetStore::OnAssetDestroyed(Asset& asset)
{
}

void AssetStore::Reload()
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
			printf("Loaded [%s] Last Modified [%s]\n",
			       asset.Name().c_str(),
			       timeString.c_str());
			asset.OnReload();
			asset.SetTimeLastModified(lastModified);
		}
	}
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