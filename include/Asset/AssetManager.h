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
			Font,
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
			*@brief Gets asset manager name
			*/
			Enjon::String GetName( );
			
			/**
			*@brief
			*/
			void SetAssetsPath( const String& filePath );
			
			/**
			*@brief
			*/
			Enjon::String GetAssetsPath( );
			
			/**
			*@brief
			*/
			void SetCachedAssetsPath( const String& filePath ); 
			
			/**
			*@brief
			*/
			const Enjon::String& GetCachedAssetsPath( ) const;
			
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

			/**
			*@brief
			*/
			bool Exists( u32 id )
			{
				return ( mLoaders.find( id ) != mLoaders.end( ) );
			}
			
			/**
			*@brief Gets all assets of specific type
			*/
			template <typename T>
			const std::unordered_map< Enjon::String, Asset* >* GetAssets( )
			{ 
				// Get appropriate loader based on asset type
				u32 loaderId = GetAssetTypeId<T>(); 

				if ( Exists( loaderId ) )
				{
					return mLoaders[ loaderId ]->GetAssets( );
				}

				return nullptr;
			} 
			
			/**
			*@brief Gets loaded asset in database from name
			*/
			template <typename T>
			AssetHandle<T> GetDefaultAsset( )
			{ 
				// Get appropriate loader based on asset type
				u32 loaderId = GetAssetTypeId<T>(); 

				// Get handle from loader
				Asset* defaultAsset = mLoaders[loaderId]->GetDefault( );
				AssetHandle<T> handle = AssetHandle<T>( defaultAsset );

				// Return asset handle
				return handle;
			} 

			/**
			*@brief 
			*/
			static bool HasFileExtension( const String& file, const String& extension );
			

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

				// TODO(): This crashes for now. I want to set this here, so figure it out.
				// Register default asset from loader
				//mLoaders[ idx ]->RegisterDefaultAsset( );

				return Result::SUCCESS;
			}

		private:
			void RegisterLoaders( );

		private:

			std::unordered_map<u32, AssetLoader*> mLoaders; 
			std::unordered_map< Enjon::String, u32 > mFileExtensionMap;
			String mAssetsPath;
			String mCachedPath;
			String mName; 
	};

	#include "Asset/AssetManager.inl"
} 


#endif
