#include <iostream>

#include <Utils/FileUtils.h>
#include <Engine.h>

#include "EditorApp.h"
 
#include <filesystem> 

#define ENJON_MEMORY_LEAK_DETECTION 0
#if ENJON_MEMORY_LEAK_DETECTION
	#include <vld.h> 
#endif

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

int main(int argc, char** argv)
{ 
	// TODO(): Hate doing it this way... just generate a config file or something...
	Enjon::String curDir = FS::current_path( ).string( );
	Enjon::String enjonDir = curDir + "/../../"; 

	Enjon::EditorApp mApp;

	for ( u32 i = 0; i < argc; ++i )
	{
		std::cout << argv[ i ] << "\n";
	}

	// Check for arguments being passed in
	if ( argc > 1 )
	{
		// Read in project file 
		String projFile = argv[ 1 ];
		
		// Read in lines of project file 
		String fileContents = Utils::read_file_sstream( argv[ 1 ] ); 

		// Grab enjon directory from contents
		Vector< String > contentSplit = Utils::SplitString( fileContents, "\n" );
		enjonDir = Utils::SplitString( fileContents, "\n" ).back( ); 

		String projectName = Enjon::Utils::SplitString( Enjon::Utils::SplitString( projFile, "\\" ).back(), "." ).front();
		mApp.SetProjectOnLoad( projectName );

		std::cout << projectName << "\n";
	}

	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

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
