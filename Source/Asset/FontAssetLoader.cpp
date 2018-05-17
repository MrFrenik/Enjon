// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/FontAssetLoader.h" 

namespace Enjon
{ 
	//======================================================================

	void FontAssetLoader::RegisterDefaultAsset( )
	{
		// Get file path to default font on disk
		String filePath = Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( ) + "/Fonts/WeblySleek/weblysleekuisb.ttf"; 

		// Load font
		Enjon::UIFont* font = new Enjon::UIFont( filePath ); 

		// Set default
		mDefaultAsset = font;
	}

	//======================================================================

	Asset* FontAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Create new font
		Enjon::UIFont* font = new Enjon::UIFont( filePath ); 

		// Return font
		return font;
	}

	//======================================================================
}
