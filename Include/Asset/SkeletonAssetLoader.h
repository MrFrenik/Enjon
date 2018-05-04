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
	ENJON_CLASS()
	class SkeletonAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief Constructor
			*/
			SkeletonAssetLoader();

			/**
			* @brief Destructor
			*/
			~SkeletonAssetLoader(); 

			/**
			* @brief 
			*/
			virtual Asset* DirectImport( const ImportOptions* options ) override; 

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
			void ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleton ); 
	}; 
}

#endif
