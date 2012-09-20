#ifndef MAIN_H
#define MAIN_H

class	DancingSquid;
struct	SDL_Surface;
union	SDL_Event;

class Main
{
private:
	SDL_Surface* 	mSurface;
	bool 			mRunning;
	DancingSquid*	mDancingSquid;

	bool ResetRenderWindow();
	void OnOpenGLContextCreated();
    void OnEvent(SDL_Event* event);
public:
	bool Reset();
	void Execute();
	Main();
	~Main();
};



/*
    //
// Reloads the settings file
//
bool Main::OnAssetReload(Asset& asset)
{


    */

#endif