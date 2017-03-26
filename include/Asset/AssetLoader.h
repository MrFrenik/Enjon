// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.h

#pragma once
#ifndef ENJON_ASSET_LOADER_H
#define ENJON_ASSET_LOADER_H 

 #include "Asset/Asset.h" 
#include "Defines.h"

#include <string>
#include <unordered_map>

namespace Enjon
{
	// Forward declaration
	class AssetManager; 

	class AssetLoader
	{
		friend AssetManager;
		
		public:
			
			/**
			* @brief Constructor - Base class for all loaders
			*/
			AssetLoader() {}

			/**
			* @brief Virtual destructor
			*/
			~AssetLoader() {}


		protected:

			/**
			* @brief Templated argument to get asset of specific type 
			*/
			template <typename T>
			AssetHandle<T> GetAsset(const String& name)
			{ 
				AssetHandle<T> handle;

				// Search through assets for name
				auto query = mAssets.find(name);
				if (query != mAssets.end()) 
				{
					handle = AssetHandle<T>(query->second); 
				}

				return handle; 
			} 
			
	protected:
			
			Asset* AddToAssets(const String& filePath, Asset* asset)
			{
				mAssets[filePath] = asset;
				return mAssets[filePath];
			}
			
			std::unordered_map<String, Asset*> mAssets;

		private:
			/**
			* @brief 
			*/
			virtual Asset* LoadAssetFromFile(const String& name) = 0; 

	};
} 

#endif
