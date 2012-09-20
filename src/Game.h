#ifndef GAME_H
#define GAME_H

#include "IAssetOwner.h"
class Asset;
class LuaState;
struct Settings;
class ManifestAssetStore;

//
// Responsible for calling the Lua update script.
//
class Game : public IAssetOwner
{
    // used as counter to determine if the lua state should be reloaded.
    unsigned int        mReloadCount;
    LuaState*           mLuaState;
    bool                mReady;
    Settings*           mSettings;
    ManifestAssetStore* mAssetStore;
public:

    Game(Settings* settings, ManifestAssetStore* assetStore);
    ~Game();

    static void Bind(LuaState* state);
    // Callbacks for Assets
    virtual bool OnAssetReload(Asset& asset);
    virtual void OnAssetDestroyed(Asset& asset);

    void ResetReloadCount() { mReloadCount = 0; }
    unsigned int GetReloadCount() { return mReloadCount; }

    // Reloads the lua state.
    void Reset();
    void Update();
    void Break() { mReady = false; }
};

#endif