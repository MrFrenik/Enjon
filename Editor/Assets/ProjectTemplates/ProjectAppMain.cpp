#include "#PROJECT_NAME.h"

#include <Engine.h>
 
#include <filesystem>

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

// An unfortunate thing that has to happen. Thanks, SDL. 
#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{ 
	#PROJECT_NAME mApp;
	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	// Grab current directory and use that as main path for application and internal engine
	Enjon::String curDir = FS::current_path( ).string( ) + "/";

	// Set root path to engine
	mConfig.SetRootPath( curDir ); 

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
