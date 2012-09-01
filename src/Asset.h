#ifndef ASSET_H
#define ASSET_H

#include <string>
#include <ctime>
#include "IAssetLoader.h"

//
// An asset is something the game uses such as a script file, texture or model
//
class Asset
{
private:
	std::string mPath;
	std::string mName;
	bool mIsLoaded;
	IAssetLoader* mLoader;
	time_t mLastModified;
public:
	Asset(const char* name, const char* path, IAssetLoader* loader);

	// Functions that trigger callbacks
	bool 				OnReload();
	void 				OnDestroy();

	// General functions
	const std::string& 	Path() const { return mPath; }
	const std::string& 	Name() const { return mName; }
	bool 				IsLoaded() const { return mIsLoaded; }
	void 				SetTimeLastModified(time_t lastModified);
	time_t 				LastModified() const { return mLastModified; }
};


#endif
