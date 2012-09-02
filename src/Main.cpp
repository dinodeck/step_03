#include "Main.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "LuaState.h"
#include "Asset.h"
#include "AssetStore.h"
#include <gl/gl.h>
#include "SDL/SDL.h"
#include "IO.h"
#include "Game.h"


Main::Main() :
 mSurface(0),
 mAssetStore(),
 mSettingsFile(NULL),
 mDeltaTime(0),
 mRunning(true),
 mGame(NULL)
{
    mSettingsFile = new Asset("settings", "settings.lua", this);
    mGame = new Game(&mSettings, &mAssetStore);
    mAssetStore.RegisterAssetOwner("scripts", mGame);
}

Main::~Main()
{
    if(mSettingsFile)
    {
        delete mSettingsFile;
    }

    if(mGame)
    {
        delete mGame;
    }
}

//
// Reloads the settings file
//
bool Main::OnAssetReload(Asset& asset)
{
    printf("-Reloading Settings-\n");
    const char* path = asset.Path().c_str();

    if(!IO::FileExists(path))
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

    ResetRenderWindow();

    if(!IO::FileExists(mSettings.manifestPath.c_str()))
    {
        printf("Manifest file doesn't exist [%s]\n", mSettings.manifestPath.c_str());
        printf("Manifest file is specified in settings.lua e.g. manifest=\"manifest.lua\"\n");
        mAssetStore.Clear();
        return false; // You can't do much without assets!
    }

    // 1. Is the asset file the same as in previous loads?
    mAssetStore.Reload(mSettings.manifestPath);

	return true;
}

void Main::OnAssetDestroyed(Asset& asset)
{

}

void Main::ReloadGame()
{
    // Should have been created in the constructor.
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

void Main::OnEvent(SDL_Event* event)
{
    switch(event->type)
    {
        case SDL_QUIT:
        {
            mRunning = false;
        } break;

        case SDL_KEYDOWN:
        {
            if(event->key.keysym.sym == SDLK_ESCAPE)
            {
                printf("Stopped looping because escape pressed.\n");
                mRunning = false;
            }
            else if(event->key.keysym.sym == SDLK_F2)
            {
                ReloadGame();
            }

        } break;
    }
}

bool Main::ResetRenderWindow()
{
    SDL_WM_SetCaption(mSettings.name.c_str(), mSettings.name.c_str());

    // SDL handles this surface memory, so it can be called multiple times without issue.
    if((mSurface = SDL_SetVideoMode(mSettings.width, mSettings.height, 32, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL)) == NULL)
    {
        printf("Error initializing graphics: %s\n", SDL_GetError());
        return false;
    }

    SDL_WarpMouse(mSettings.width/2, mSettings.height/2);


    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, mSettings.width, mSettings.height);
     // Setups an orthographic view, should be handled by renderer.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, mSettings.width, mSettings.height, 0, 1, -1);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glClearColor(0.0, 0.0, 0.0, 1.0f);

    // Enabled blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    return true;
}

void Main::OnOpenGLContextCreated()
{

    ReloadGame();
    // Setup the window
    ResetRenderWindow();
}


void Main::Execute()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
    	printf("SDL Failed to init");
        return;
    }

    // Allow the game pads to be polled.
    SDL_JoystickEventState(SDL_IGNORE);
    SDL_EnableUNICODE(1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,      8);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,     32);

    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,  	8);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 	8);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

    OnOpenGLContextCreated();

    unsigned int thisTime = 0;
    unsigned int lastTime = 0;
    unsigned int framesPerSecond = 60;
    unsigned int millisecondsPerFrame = 1000 / framesPerSecond;
    unsigned int fpsTicks = 0;

    SDL_Event event;

    while(mRunning)
    {
        // Calculate delta time
        thisTime = SDL_GetTicks(); // returns in milliseconds
        mDeltaTime = static_cast<double>((thisTime - lastTime) / 1000); // convert to seconds
        lastTime = thisTime;

        while(SDL_PollEvent(&event))
        {
            OnEvent(&event);
        }

    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        OnUpdate();

		fpsTicks = SDL_GetTicks() - fpsTicks;
        if (fpsTicks < millisecondsPerFrame)
        {
            SDL_Delay(millisecondsPerFrame - fpsTicks);
        }
    	SDL_GL_SwapBuffers();
    }

    SDL_Quit();

	return;
}

void Main::OnUpdate()
{
	// Game code goes here.
    mGame->Update();
}

int main(int argc, char *argv[]){
	Main main;
	main.Execute();
	return 0;
}