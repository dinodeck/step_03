#ifndef DANCINGSQUID_H
#define DANCINGSQUID_H
#include <string>

#include "AssetStore.h"
#include "IAssetOwner.h"
#include "ManifestAssetStore.h"
#include "Settings.h"

// Dancing Squid is the kernal of the engine and should have limited dependancies.
// It assumes access to OpenGL

class Asset;
class Game;

class DancingSquid : IAssetOwner
{
private:
    std::string mName; // name of the game / project
    ManifestAssetStore  mAssetStore;
    Settings mSettings;
    Asset*  mSettingsFile;
    Game*   mGame;
public:
    DancingSquid(const std::string& name);
    ~DancingSquid();
    void Update(double deltaTime);
    const std::string& Name() const { return mName; }
    void SetName(const std::string& value) { mName = value; }
    unsigned int ViewWidth() const { return mSettings.width; }
    unsigned int ViewHeight() const { return mSettings.height; }
    void ResetRenderWindow(unsigned int width, unsigned int height);
    // Callback to reload the manifest
    virtual void OnAssetDestroyed(Asset& asset);
    virtual bool OnAssetReload(Asset& asset);
    void ForceReload();
};

#endif
