// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.h

#pragma  once
#ifndef ENJON_MESH_ASSET_LOADER_H
#define ENJON_MESH_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/Mesh.h"

namespace Enjon
{
	ENJON_CLASS()
	class MeshAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY()

		public:

			/**
			* @brief Constructor
			*/
			MeshAssetLoader();

			/**
			* @brief Destructor
			*/
			~MeshAssetLoader(); 

		protected:

			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override;
			
		private: 
			/**
			* @brief Destructor
			*/
			virtual Asset* LoadResourceFromFile(const String& filePath, const String& name) override;
	}; 
}

#endif
