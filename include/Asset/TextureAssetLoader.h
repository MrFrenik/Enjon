// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.h

#pragma  once
#ifndef ENJON_TEXTURE_ASSET_LOADER_H
#define ENJON_TEXTURE_ASSET_LOADER_H 

#include "Base/Object.h"
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

		public:
			/**
			* @brief
			*/
			static Result CacheTextureData( const u8* data, u32 length, Texture* texture ); 
			
		protected:
			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override;

		private: 
			/**
			* @brief
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath, const String& name ) override;
	
			/**
			* @brief
			*/
			Texture* LoadTextureFromFile( const Enjon::String& filePath );
	}; 
}

#endif
