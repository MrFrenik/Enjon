// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.h

#pragma  once
#ifndef ENJON_MESH_ASSET_LOADER_H
#define ENJON_MESH_ASSET_LOADER_H 

#include "Asset/ImportOptions.h"
#include "Asset/AssetLoader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Skeleton.h"

// Assimp specifics
struct aiMesh;
struct aiNode;
struct aiScene;

namespace Enjon
{
	// Forward Declarations
	class MeshAssetLoader;
	class SkeletalAnimation;
	class SkeletonAssetLoader;
	class SkeletalMeshAssetLoader;
	class SkeletalAnimationAssetLoader;

	class MeshImportOptions : public ImportOptions
	{
		friend SkeletonAssetLoader;
		friend SkeletalMeshAssetLoader;
		friend SkeletalAnimationAssetLoader;
		friend MeshAssetLoader;

		public:

			/*
			* @brief
			*/
			MeshImportOptions( ) = default;

			/*
			* @brief
			*/
			~MeshImportOptions( ) = default;

			/*
			* @brief
			*/
			AssetHandle< Skeleton > GetSkeleton( ) const;

		protected:

			/*
			* @brief
			*/
			virtual Result OnEditorUIInternal( ) override; 

		protected: 
			u32 mShowSkeletonCreateDialogue : 1;
			u32 mCreateSkeleton : 1;
			u32 mShowMeshCreateDialogue : 1;
			u32 mCreateMesh : 1;
			u32 mShowAnimationCreateDialogue : 1;
			u32 mCreateAnimations : 1;
			AssetHandle< Skeleton > mSkeletonAsset;
	};

	ENJON_CLASS( )
	class MeshAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( MeshAssetLoader )

		public:

			/**
			* @brief Constructor
			*/
			void virtual ExplicitConstructor() override; 

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

			/**
			* @brief
			*/
			virtual const ImportOptions* GetImportOptions( ) const override;

			/**
			* @brief
			*/
			virtual String GetAssetFileExtension( ) const override;

		protected:

			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override;

			/**
			* @brief
			*/
			virtual Result BeginImportInternal( const String& filepath, const String& cacheDirectory ) override; 
			
		private: 
			/**
			* @brief Destructor
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath ) override; 

			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromImporter( const ImportOptions* options ) override;

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

			void ProcessSkeletalMesh( aiMesh* aim, const aiScene* scene, Skeleton* skeleton, Mesh* mesh, Vector< VertexJointData >* vertexJointData );

			void ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleon, Mesh* mesh, Vector< VertexJointData >* vertexJointData ); 

			void ProcessAnimNode( aiNode* node, const aiScene* scene, Skeleton* skeleton, SkeletalAnimation* animation );

			void BuildBoneHeirarchy( const aiNode* node, const aiNode* parent, Skeleton* skeleton );

			Vector< Skeleton* > mSkeletons;
			Vector< SkeletalAnimation* > mAnimations;
			MeshImportOptions mImportOptions;
	}; 
}

#endif
