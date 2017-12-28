#include <iostream>

#include <Engine.h>

#include "EditorApp.h"

int main(int argc, char** argv)
{ 
	EnjonEditor mEditor;
	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	// Set root path to engine
	mConfig.SetRootPath( "E:/Development/Enjon/" );

	// Startup engine
	Enjon::Result res = mEngine.StartUp( &mEditor, mConfig ); 

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
