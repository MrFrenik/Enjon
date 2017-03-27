// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.h

#pragma  once
#ifndef ENJON_MESH_ASSET_LOADER_H
#define ENJON_MESH_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/Mesh.h"

namespace Enjon
{
	class MeshAssetLoader : public AssetLoader
	{
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

		private: 
			Mesh* LoadAssetFromFile(const String& filePath, const String& name);
	}; 
}

#endif
