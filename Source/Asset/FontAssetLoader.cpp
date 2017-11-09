// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/FontAssetLoader.h" 

namespace Enjon
{
	//======================================================================

	FontAssetLoader::FontAssetLoader( )
	{
	}

	//======================================================================

	FontAssetLoader::~FontAssetLoader( )
	{
	}

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

	Asset* FontAssetLoader::LoadResourceFromFile( const String& filePath, const String& name )
	{
		// Create new font
		Enjon::UIFont* font = new Enjon::UIFont( filePath ); 

		// Add to assets with qualified name
		AddToAssets( name, font );

		// Return font
		return font;
	}

	//======================================================================
}
