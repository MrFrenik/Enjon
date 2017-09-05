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
			virtual void RegisterDefaultAsset( )
			{ 
			}
			
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

				// If found, then return asset
				if ( query != mAssetsByName.end() ) 
				{
					handle = AssetHandle<T>( query->second ); 
				}
				// Else return default asset
				else
				{
					handle = AssetHandle<T>( GetDefault( ) );
				}

				return handle; 
			} 
			
			/**
			* @brief Templated argument to get asset of specific type 
			*/
			Asset* GetDefault( )
			{ 
				if ( mDefaultAsset )
				{
					return mDefaultAsset;
				}
				
				// If not registered, then register and return default
				RegisterDefaultAsset( ); 
				return mDefaultAsset;
			} 

			/**
			* @brief
			*/
			bool HasAsset( const String& name )
			{
				return ( mAssetsByName.find( name ) != mAssetsByName.end( ) );
			} 

			const std::unordered_map< String, Asset* >* GetAssets( )
			{
				return &mAssetsByName;
			}
			
			/**
			* @brief
			*/
			Asset* AddToAssets( const String& name, Asset* asset )
			{
				if ( HasAsset( name ) )
				{
					return mAssetsByName[ name ];
				} 

				// Set name
				asset->mName = name;

				// Otherwise add asset
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
			Asset* mDefaultAsset = nullptr;

		private:
			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath, const String& name ) = 0; 
	};
} 

#endif
