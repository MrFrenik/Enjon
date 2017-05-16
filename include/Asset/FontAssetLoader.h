// @file: FontAssetLoader.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma  once
#ifndef ENJON_FONT_ASSET_LOADER_H
#define ENJON_FONT_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/Font.h"

namespace Enjon
{
	class FontAssetLoader : public AssetLoader
	{
	public:

		/**
		* @brief Constructor
		*/
		FontAssetLoader( );

		/**
		* @brief Destructor
		*/
		~FontAssetLoader( );

	protected:

	private:
		UIFont* LoadAssetFromFile( const String& filePath, const String& name );
	};
}

#endif
