#include "Game.h"
#include "IAssetOwner.h"
#include "Asset.h"
#include "LuaState.h"
#include "Settings.h"
#include "AssetStore.h"

Game::Game(Settings* settings, AssetStore* assetStore) :
    mReloadCount(0),
    mLuaState(NULL),
    mReady(false),
    mSettings(settings),
    mAssetStore(assetStore)
{
    mLuaState = new LuaState("Game");
}

Game::~Game()
{
    if(mLuaState)
    {
        delete mLuaState;
        mLuaState = NULL;
    }
}

bool Game::OnAssetReload(Asset& asset)
{

    printf("Script asset should be reloaded: [%s]\n", asset.Name().c_str());
    mReloadCount++;
    return true;
}

void Game::OnAssetDestroyed(Asset& asset)
{
    // If you remove a script, that means we'll need to reload.
    mReloadCount++;
}

 void Game::Reset()
 {
    printf("Going to reload the lua state.\n");
    mLuaState->Reset();

    // Main Script Id, needs to be taken from the asset store.
    // Settings->mainScript
    const char* mainScriptName = mSettings->mainScript.c_str();
    if(!mAssetStore->AssetExists(mainScriptName))
    {
        printf("Main script [%s], definied in settings.lua, does not exist in asset store.\n",
               mainScriptName);
        mReady = false;
        return;
    }

    Asset* mainScript = mAssetStore->GetAssetByName(mainScriptName);
    printf("Calling RunScript %s", mainScriptName);
    if(mLuaState->DoFile(mainScript->Path().c_str()))
    {
        printf("Reload success:\n");
        mReady = true;
    }
    else
    {
        printf("Reload failed:");
        printf("\tPress F2 to reload.\n");
        mReady = false;
    }
 }

 void Game::Update()
 {
    if(!mReady)
    {
        return;
    }

    // // // Update the input
    // // for(std::vector<Pad*>::iterator it = mGamePads.begin(); it != mGamePads.end(); ++it)
    // // {
    // //     (*it)->Update();
    // // }
    // mouse.Update(deltaTime);
    // keyboard.Update(deltaTime);


    // // This should be in the render function
    // for(std::vector<Renderer*>::iterator it = Renderer::mRenderers.begin(); it !=Renderer::mRenderers.end(); ++it)
    // {
    //     (*it)->Render();
    // }

    bool result = mLuaState->DoString("update()");

    if(result == 1)
    {
        printf("Press F2 to reload.\n");
        mReady = false;
    }

    // Force a full collect each frame
    mLuaState->CollectGarbage();

 }