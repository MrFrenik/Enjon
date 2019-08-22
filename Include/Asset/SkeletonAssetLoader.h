// @file SkeletonAssetLoader.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SKELETON_ASSET_LOADER_H
#define ENJON_SKELETON_ASSET_LOADER_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Asset/AssetLoader.h"
#include "Graphics/Skeleton.h"

// Assimp specifics
struct aiMesh;
struct aiNode;
struct aiScene;

namespace Enjon
{
	ENJON_CLASS( )
	class SkeletonAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( SkeletonAssetLoader )

		public: 

			/**
			* @brief 
			*/
			virtual Asset* DirectImport( const ImportOptions* options ) override; 

			/**
			* @brief 
			*/
			virtual String SkeletonAssetLoader::GetAssetFileExtension( ) const override;

			static void DecomposeMatrix( const Mat4x4& original, Vec3& position, Vec3& scale, Quaternion& rotation );

		protected:

			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

		private:

			/**
			* @brief
			*/
			void BuildBoneHeirarchy( const aiNode* node, const aiNode* parent, Skeleton* skeleton );

			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromImporter( const ImportOptions* options ) override;

			/**
			* @brief 
			*/

			void ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleton, u32 indent );
			//void ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleton ); 

			void DisplaySkeletonHeirarchyRecursive( const Skeleton* skeleton, u32 index, u32 indent );
	}; 
}

#endif
