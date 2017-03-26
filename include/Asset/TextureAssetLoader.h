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

		protected:

		private: 
			Texture* LoadAssetFromFile(const std::string& name);
	}; 
}

#endif
