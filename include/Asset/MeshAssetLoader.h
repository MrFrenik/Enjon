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
	class SkeletalAnimation;

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

			// NOTE(): Total temporary
			Vector< Skeleton* > GetSkeletons( )
			{
				return mSkeletons;
			}

			// NOTE(): Total temporary
			SkeletalAnimation* GetAnimation( const u32& index ) const
			{
				if ( index >= mAnimations.size( ) )
				{
					return nullptr;
				}

				return mAnimations.at( index );
			}

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

			bool HasMesh( aiNode* node, const aiScene* scene );

			void ProcessSkeletalMesh( aiMesh* aim, const aiScene* scene, Skeleton* skeleton, Mesh* mesh );

			void ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleon, Mesh* mesh ); 

			void ProcessAnimNode( aiNode* node, const aiScene* scene, Skeleton* skeleton, SkeletalAnimation* animation );

			void BuildBoneHeirarchy( const aiNode* node, const aiNode* parent, Skeleton* skeleton );

			Vector< Skeleton* > mSkeletons;
			Vector< SkeletalAnimation* > mAnimations;
	}; 
}

#endif
