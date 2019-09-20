
#include "SandboxApp.h"

#include <Graphics/Window.h>
#include <Utils/FileUtils.h>
#include <Engine.h> 
 
#include <fs/filesystem.hpp> 

// TODO(john): Have to figure out a good way of including this into the repo. For now, will just comment out. 
//#define ENJON_MEMORY_LEAK_DETECTION 0
//#if ENJON_MEMORY_LEAK_DETECTION
//	#include <vld.h> 
//#endif

namespace fs = ghc::filesystem; 

using namespace Enjon; 

#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{ 
#ifdef ENJON_SYSTEM_WINDOWS
	// Hide console window for now ( kinda jank; need to change the subsystem instead )
	//HWND hWnd = GetConsoleWindow();
	//ShowWindow( hWnd, SW_HIDE ); 
#endif

	// TODO(): Hate doing it this way... just generate a config file or something...
	Enjon::String curDir = fs::current_path( ).string( );
	Enjon::String enjonDir = curDir + "/../../"; 
	String projDir = "";
	String projName = "";
 
	assert( argc > 2 ); 

	projDir = argv[ 1 ]; 
	projName = argv[ 2 ];

	SandboxApp mApp( projDir );
	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	WindowParams params;
	params.mFlags = WindowFlags::RESIZABLE;
	params.mMetaClassFunc = [&]() -> const MetaClass * { return Object::GetClass< Window >(); };
	params.mWidth = 900;
	params.mHeight = 506;
	params.mName = "Sandbox: " + projName;

	// Set root path to engine
	mConfig.SetRootPath( enjonDir ); 
	// Set main window params for config
	mConfig.SetMainWindowParams( &params ); 
	// Just for shiggles( no gui )
	mConfig.SetIsStandAloneApplication( true );
 
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
