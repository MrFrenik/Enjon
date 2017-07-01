// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.h

#pragma  once
#ifndef ENJON_TEXTURE_ASSET_LOADER_H
#define ENJON_TEXTURE_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/Texture.h"

namespace Enjon
{
	class TextureAssetLoader : public AssetLoader
	{
		public:

			/**
			* @brief Constructor
			*/
			TextureAssetLoader();

			/**
			* @brief Destructor
			*/
			~TextureAssetLoader(); 
			
			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override;

		protected:
			/**
			* @brief
			*/
			Result CacheTextureData( const u8* data, u32 length, Texture* texture );

		private: 
			Texture* LoadAssetFromFile( const String& filePath, const String& name );
	
			Texture* LoadTextureFromFile( const Enjon::String& filePath );
	}; 
}

#endif
