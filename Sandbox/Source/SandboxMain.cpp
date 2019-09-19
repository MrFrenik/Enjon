
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
	// TODO(): Hate doing it this way... just generate a config file or something...
	Enjon::String curDir = fs::current_path( ).string( );
	Enjon::String enjonDir = curDir + "/../../"; 
	String projDir = "";
 
	assert( argc > 1 ); 

	projDir = argv[ 1 ]; 
	SandboxApp mApp( projDir );
	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	WindowParams params;
	params.mFlags = WindowFlags::RESIZABLE;
	params.mMetaClassFunc = [&]() -> const MetaClass * { return Object::GetClass< Window >(); };
	params.mWidth = 800;
	params.mHeight = 600;
	params.mName = "Sandbox";

	// Set root path to engine
	mConfig.SetRootPath( enjonDir ); 
	// Set main window params for config
	mConfig.SetMainWindowParams( &params ); 
 
	// Startup engine
	Enjon::Result res = mEngine.StartUp( &mApp, mConfig ); 

	// If successful startup, then run the engine
	if ( res == Enjon::Result::SUCCESS )
	{
		std::cout << "Got here!\n";
		mEngine.Run( );
	} 
	else 
	{ 
		// Print error...
	}

	std::cout << "Leaving!\n";
	
	return 0;
}
