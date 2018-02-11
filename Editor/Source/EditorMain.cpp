#include <iostream>

#include <Engine.h>

#include "EditorApp.h"

#include <filesystem> 
#include <vld.h>

namespace FS = std::experimental::filesystem;

struct Thing
{
	Enjon::f32 value = 1.0f;
};

int main(int argc, char** argv)
{ 
	Enjon::EditorApp mApp;
	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	// TODO(): Hate doing it this way... just generate a config file or something...
	Enjon::String curDir = FS::current_path( ).string( );
	Enjon::String enjonDir = curDir + "/../../"; 

	// Set root path to engine
	mConfig.SetRootPath( enjonDir );

	// Startup engine
	Enjon::Result res = mEngine.StartUp( &mApp, mConfig ); 

	// If successful startup, then run the engine
	if ( res == Enjon::Result::SUCCESS )
	{
		mEngine.Run( );
	} 
	else 
	{ 
		// Print error...
	}
	
	return 0;
}
