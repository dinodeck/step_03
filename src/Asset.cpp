#include "Asset.h"
#include <assert.h>
#include <stdio.h>

bool Asset::OnReload()
{
	assert(mLoader);
	mIsLoaded = mLoader->OnAssetReload(*this);
    return mIsLoaded;
}

//
// Gives a chance for the owner to clean up the Asset
//
void Asset::OnDestroy()
{
    assert(mLoader);
    mLoader->OnAssetDestroyed(*this);
}

Asset::Asset(const char* name, const char* path, IAssetOwner* owner)
{
	assert(owner);
	mName = name;
	mPath = path;
	mLoader = owner;
	mIsLoaded = false;
}

void Asset::SetTimeLastModified(time_t lastModified)
{
	mLastModified = lastModified;
}