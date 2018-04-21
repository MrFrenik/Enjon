// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.h

#pragma  once
#ifndef ENJON_MESH_ASSET_LOADER_H
#define ENJON_MESH_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Skeleton.h"

// Assimp specifics
struct aiMesh;
struct aiNode;
struct aiScene;

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
			virtual Asset* LoadResourceFromFile(const String& filePath ) override; 

			/**
			* @brief
			*/
			void ProcessNode( aiNode* node, const aiScene* scene, Mesh* mesh );

			/**
			* @brief
			*/
			void ProcessMesh( aiMesh* aim, const aiScene* scene, Mesh* mesh );

			bool HasSkeleton( aiNode* node, const aiScene* scene );

			void ProcessSkeletalMesh( aiMesh* aim, const aiScene* scene, Skeleton* skeleton, Mesh* mesh );

			void ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleon, Mesh* mesh );

			void ProcessAnimation( aiNode* node, const aiScene* scene ); 
	}; 
}

#endif
