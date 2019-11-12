// @file: FontAssetLoader.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma  once
#ifndef ENJON_FONT_ASSET_LOADER_H
#define ENJON_FONT_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/Font.h"

namespace Enjon
{
	ENJON_CLASS( )
	class FontAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( FontAssetLoader ) 
 
		protected:
			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

			/**
			* @brief
			*/
			String GetAssetFileExtension( ) const;

		private:
			Asset* LoadResourceFromFile( const String& filePath ) override;
	};
}

#endif
