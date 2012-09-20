#include "DancingSquid.h"

#include <assert.h>
#include <cstdio>

#include "Asset.h"
#include "DancingSquidGL.h"
#include "Game.h"
#include "LuaState.h"
#include "DSFile.h"

DancingSquid::DancingSquid(const std::string& name)
    :   mName(name),
        mAssetStore(),
        mSettings(),
        mSettingsFile(NULL),
        mGame(NULL)

{
    mSettingsFile = new Asset("settings", "settings.lua", this);
    mGame = new Game(&mSettings, &mAssetStore);
    mAssetStore.RegisterAssetOwner("scripts", mGame);
}

DancingSquid::~DancingSquid()
{

}

bool DancingSquid::OnAssetReload(Asset& asset)
{
    printf("-Reloading Settings-\n");
    const char* path = asset.Path().c_str();

    if(!DSFile::FileExists(path))
    {

        printf("Settings file [%s] doesn't exist.\n", path);

        // If the settings file doesn't exist, then the assets
        // should be cleared. As they're no longer in the project.
        mAssetStore.Clear();

        return false;
    }

    //
    // Create a lua state to parse the settings file
    //
    LuaState luaState("Settings");
    bool success = luaState.DoFile(path);

    if(!success)
    {
        printf("Lua failed to parse settings [%s].\n", path);
        // For now don't clear the asset store.
        return false;
    }

    // Get values, or assign defaults if not present.
    mSettings.name = luaState.GetString("name", mSettings.name.c_str());
    mSettings.width = luaState.GetInt("width", mSettings.width);
    mSettings.height = luaState.GetInt("height", mSettings.height);
    mSettings.mainScript = luaState.GetString("main_script", "main.lua");
    mSettings.onUpdate = luaState.GetString("on_update", "update()");
    mSettings.manifestPath = luaState.GetString("manifest", "");
    SetName(mSettings.name);
    ResetRenderWindow(mSettings.width,  mSettings.height);

    if(!DSFile::FileExists(mSettings.manifestPath.c_str()))
    {
        printf("Manifest file doesn't exist [%s]\n", mSettings.manifestPath.c_str());
        printf("Manifest file is specified in settings.lua e.g. manifest=\"manifest.lua\"\n");
        mAssetStore.Clear();
        return false; // You can't do much without assets!
    }

    // 1. Is the asset file the same as in previous loads?
    printf("Calling reload [%s].\n", mSettings.manifestPath.c_str());
    mAssetStore.Reload(mSettings.manifestPath);

    return true;
}

void DancingSquid::OnAssetDestroyed(Asset& asset)
{
    // Nothing to do.
}

void DancingSquid::ForceReload()
{
    assert(mSettingsFile);
    mGame->ResetReloadCount();

    if( AssetStore::IsOutOfDate(*mSettingsFile) )
    {
        bool success = mSettingsFile->OnReload();

        if(success)
        {
            // Update the timestamp
            time_t lastModified = AssetStore::GetModifiedTimeStamp(*mSettingsFile);
            mSettingsFile->SetTimeLastModified(lastModified);
        }
    }
    else
    {
        printf("[%s] SKIPPED.\n", mSettingsFile->Path().c_str());
        // But that doesn't mean the manifest or other files haven't
        mAssetStore.Reload();
    }

    if(mGame->GetReloadCount() > 0)
    {
        mGame->Reset();
    }
}

//
// @deltaTime Number of seconds last frame took
//              * Capped to 1/60 on Windows
void DancingSquid::Update(double deltaTime)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mGame->Update();
}

void DancingSquid::ResetRenderWindow(unsigned int width, unsigned int height)
{
    mSettings.width = width;
    mSettings.height = height;
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, mSettings.width, mSettings.height);

     // Setups an orthographic view, should be handled by renderer.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0, mSettings.width, mSettings.height, 0, 1, -1);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glClearColor(0.0, 1.0, 1.0, 1.0f);

    // Enabled blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}