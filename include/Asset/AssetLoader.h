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
			AssetLoader();

			/**
			* @brief Destructor
			*/
			~AssetLoader();

			/**
			* @brief 
			*/ 
			static String GetQualifiedName( const String& filePath );

			/**
			* @brief
			*/
			bool Exists( const String& name );
			
			/**
			* @brief
			*/
			bool Exists( UUID uuid );

		protected:

			/**
			* @brief Templated argument to get asset of specific type 
			*/
			template <typename T>
			AssetHandle<T> GetAsset( const String& name )
			{ 
				AssetHandle<T> handle;

				// Search through assets for name
				auto query = mAssetsByName.find( name );
				if ( query != mAssetsByName.end() ) 
				{
					handle = AssetHandle<T>( query->second ); 
				}

				return handle; 
			} 
			
			/**
			* @brief
			*/
			Asset* AddToAssets( const String& name, Asset* asset )
			{
				mAssetsByName[name] = asset;
				mAssetsByUUID[ asset->mUUID.ToString( ) ] = asset;

				return mAssetsByName[name];
			}

			/**
			* @brief
			*/
			virtual Result CacheFile( Enjon::ByteBuffer& buffer )
			{
				return Enjon::Result::SUCCESS;
			}

	protected:
			
			std::unordered_map<String, Asset*> mAssetsByName;
			std::unordered_map<String, Asset*> mAssetsByUUID;

		private:
			/**
			* @brief 
			*/
			virtual Asset* LoadAssetFromFile( const String& filePath, const String& name ) = 0; 
	};
} 

#endif
