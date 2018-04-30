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
	}; 
}

#endif
