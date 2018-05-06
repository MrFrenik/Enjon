// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MaterialAssetLoader.h

#pragma  once
#ifndef ENJON_MATERIAL_ASSET_LOADER_H
#define ENJON_MATERIAL_ASSET_LOADER_H 

#include "Graphics/Material.h"
#include "Asset/AssetLoader.h"

namespace Enjon
{
	ENJON_CLASS( )
	class MaterialAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( )
		
		public:

			/**
			* @brief Constructor
			*/
			MaterialAssetLoader( ) = default;

			/**
			* @brief Destructor
			*/
			~MaterialAssetLoader( ) = default;

			/**
			* @brief Destructor
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
			Asset* LoadResourceFromFile( const String& filePath ) override; 
	};

}

#endif
