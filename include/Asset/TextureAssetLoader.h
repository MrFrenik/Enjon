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
	ENJON_CLASS( )
	class TextureAssetLoader : public AssetLoader
	{ 
		ENJON_CLASS_BODY( TextureAssetLoader )

		public: 

			/**
			* @brief
			*/
			virtual String GetAssetFileExtension( ) const override;
			
		protected:
			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

		private: 
			/**
			* @brief
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath ) override;
	
			/**
			* @brief
			*/
			Texture* LoadTextureFromFile( const Enjon::String& filePath );
	}; 
}

#endif
