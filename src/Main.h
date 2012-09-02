#ifndef MAIN_H
#define MAIN_H
#include <string>
#include "IAssetOwner.h"
#include "AssetStore.h"
#include "Settings.h"


struct SDL_Surface;
union SDL_Event;

class Asset;
class Game;

class Main : IAssetOwner
{
private:
	Settings 		mSettings;
	SDL_Surface* 	mSurface;
	AssetStore 		mAssetStore;
	Asset* 			mSettingsFile;
	double 			mDeltaTime;
	bool 			mRunning;
    Game*           mGame;

    void OnEvent(SDL_Event* event);
	void OnUpdate();
	bool ResetRenderWindow();
	void OnOpenGLContextCreated();
	void ReloadGame();
public:
	// Callback to reload the settings file
	virtual bool OnAssetReload(Asset& asset);
	virtual void OnAssetDestroyed(Asset& asset);
	void Execute();
	bool Reset();
	Main();
	~Main();
};

#endif