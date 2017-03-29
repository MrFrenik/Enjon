// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file AssetManager.h
#pragma once
#ifndef ENJON_ASSET_MANAGER_H
#define ENJON_ASSET_MANAGER_H 

#include "Subsystem.h"
#include "System/Types.h"
#include "Asset/AssetLoader.h"
#include "Graphics/Texture.h"
#include "Defines.h" 

#include <array>

namespace Enjon
{
	class AssetLoader;
	class Asset; 

	class AssetManager : public Subsystem
	{
		enum class LoaderType
		{
			Texture,
			Material,
			Mesh,
			Shader,
			Count
		}; 

		public: 
			
			/**
			*@brief Constructor
			*/
			AssetManager();

			/**
			*@brief Constructor
			*/
			AssetManager(const String& name, const String& assetsPath);

			/**
			*@brief Virtual destructor
			*/
			~AssetManager();

			/**
			*@brief
			*/
			virtual Result Initialize();

			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) override;

			/**
			*@brief
			*/
			virtual Result Shutdown();
	
			/**
			*@brief Adds asset to project form given file path
			*/
			Result AddToDatabase( const String& filePath, b8 isRelativePath = true );
			
			/**
			*@brief
			*/
			void SetAssetsPath( const String& filePath );
			
			/**
			*@brief
			*/
			void SetDatabaseName( const String& name );

			/**
			*@brief Gets loaded asset in database from name
			*/
			template <typename T>
			AssetHandle<T> GetAsset( const String& name )
			{ 
				// Get appropriate loader based on asset type
				u32 loaderId = GetAssetTypeId<T>(); 

				// Get handle from loader
				AssetHandle<T> handle = mLoaders[loaderId]->GetAsset<T>(name); 

				// Return asset handle
				return handle;
			} 

		protected:

		private: 

			/**
			*@brief
			*/
			s32 GetLoaderIdxByFileExtension(const String& filePath);

			/**
			*@brief
			*/
			u32 GetUniqueAssetTypeId() noexcept
			{
				static u32 lastId{ 0u };
				return lastId++;
			} 

			/**
			*@brief
			*/
			template <typename T>
			u32 GetAssetTypeId() noexcept
			{
				static_assert(std::is_base_of<Asset, T>::value,
					"GetAssetTypeId:: T must inherit from Asset.");

				static u32 typeId{ GetUniqueAssetTypeId() };
				return typeId;
			}

			/**
			*@brief
			*/
			template <typename T, typename K>
			Enjon::Result RegisterAssetLoader()
			{
				static_assert(std::is_base_of<Asset, T>::value,
					"RegisterAssetLoader: T must inherit from Asset.");
				
				static_assert(std::is_base_of<AssetLoader, K>::value,
					"RegisterAssetLoader: K must inherit from AssetLoader.");

				// Get idx of asset loader type
				u32 idx = GetAssetTypeId<T>(); 

				// Set into map
				mLoaders[idx] = new K();

				return Result::SUCCESS;
			}

		private:

			std::unordered_map<u32, AssetLoader*> mLoaders; 
			String mAssetsPath;
			String mName; 
	};

	#include "Asset/AssetManager.inl"
} 


#endif
