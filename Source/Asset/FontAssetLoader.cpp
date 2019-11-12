// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/FontAssetLoader.h" 

namespace Enjon
{ 
	//======================================================================

	void FontAssetLoader::RegisterDefaultAsset( )
	{
		// Get file path to default font on disk ( don't like this, but it is what it is... )
		String filePath = Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( ) + "/Fonts/WeblySleek/weblysleekuisb.ttf"; 

		// Load font
		UIFont* font = new UIFont( filePath ); 

		// Set default
		mDefaultAsset = font;
	}

	//======================================================================

	Asset* FontAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Create new font
		UIFont* font = new UIFont( filePath ); 

		// Return font
		return font;
	} 

	//=====================================================================================================

	String FontAssetLoader::GetAssetFileExtension( ) const
	{
		return ".efnt";
	}

	//=====================================================================================================
}
