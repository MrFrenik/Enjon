// File: SceneAssetLoader.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma  once
#ifndef ENJON_SCENE_ASSET_LOADER_H
#define ENJON_SCENE_ASSET_LOADER_H 

#include "Scene/Scene.h"
#include "Asset/AssetLoader.h"

namespace Enjon
{
	ENJON_CLASS( )
	class SceneAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( )
		
		public:

			/**
			* @brief Constructor
			*/
			SceneAssetLoader( ) = default;

			/**
			* @brief Destructor
			*/
			~SceneAssetLoader( ) = default;

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
			Asset* LoadResourceFromFile( const String& filePath ) override; 
	};

}

#endif
