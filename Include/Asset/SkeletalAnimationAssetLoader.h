// @file SkeletalAnimationAssetLoader.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SKELETAL_ANIMATION_ASSET_LOADER_H
#define ENJON_SKELETAL_ANIMATION_ASSET_LOADER_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Asset/AssetLoader.h"
#include "Graphics/SkeletalAnimation.h"

// Assimp specifics
struct aiMesh;
struct aiNode;
struct aiScene;

namespace Enjon
{
	class Skeleton;

	ENJON_CLASS()
	class SkeletalAnimationAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief Constructor
			*/
			SkeletalAnimationAssetLoader();

			/**
			* @brief Destructor
			*/
			~SkeletalAnimationAssetLoader(); 

			/**
			* @brief 
			*/
			virtual Asset* DirectImport( const ImportOptions* options ) override;

		protected:

			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromImporter( const ImportOptions* options ) override;

		private:

			/**
			* @brief 
			*/
			void ProcessAnimNode( aiNode* node, const aiScene* scene, const AssetHandle< Skeleton >& skeleton, SkeletalAnimation* animation );

	}; 
}

#endif
