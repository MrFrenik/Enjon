// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/FontAssetLoader.h" 

namespace Enjon
{
	FontAssetLoader::FontAssetLoader( )
	{
	}

	FontAssetLoader::~FontAssetLoader( )
	{
	}

	UIFont* FontAssetLoader::LoadAssetFromFile( const String& filePath, const String& name )
	{
		// Create new font
		Enjon::UIFont* font = new Enjon::UIFont( filePath ); 

		// Add to assets with qualified name
		AddToAssets( name, font );

		// Return font
		return font;
	}
}
