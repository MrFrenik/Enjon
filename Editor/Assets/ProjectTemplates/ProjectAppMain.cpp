#include "#PROJECT_NAME.h"

#include <Engine.h>
 
#include <filesystem>

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

int main(int argc, char** argv)
{ 
	#PROJECT_NAME mApp;
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
