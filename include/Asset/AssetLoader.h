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
	ENJON_ENUM( )
	enum class AssetState
	{
		Unloaded,				// When there is no record of the asset at all
		Preloaded,				// Cache record is created
		Loaded					// Fully loaded into memory and ready to use
	};

	class AssetLoader;
	class AssetRecordInfo
	{
		friend AssetLoader;

		public:

			AssetRecordInfo( ) = default;

			AssetRecordInfo( Asset* asset )
				: mAsset( asset )
			{
			} 

			~AssetRecordInfo( ) = default;

			const Asset* GetAsset( ) const
			{
				return mAsset;
			}

			String GetAssetName( ) const
			{
				if ( mAsset )
				{
					return mAsset->GetName( ); 
				}
				return "UnloadedAsset";
			}

		private:
			Asset* mAsset = nullptr; 
			String mResourcePath = "";
			AssetState mAssetState = AssetState::Unloaded;
	}; 

	// Forward declaration
	class AssetManager; 
 
	ENJON_CLASS( )
	class AssetLoader : public Enjon::Object 
	{ 
		friend AssetManager; 

		// Don't like this here, but apparently I need it or the linker freaks out...
		ENJON_CLASS_BODY( )
		
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
			* @brief Returns default asset. Will register if not available yet.
			*/
			Asset* GetDefaultAsset( ); 
			
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
					handle = AssetHandle<T>( query->second.mAsset ); 
				}
				// Else return default asset
				else
				{
					handle = AssetHandle<T>( GetDefault( ) );
				}

				return handle; 
			} 

			/**
			* @brief 
			*/
			Asset* GetAsset( UUID id );
			
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
			bool HasAsset( const String& name ) const
			{
				return ( mAssetsByName.find( name ) != mAssetsByName.end( ) );
			} 

			const HashMap< String, AssetRecordInfo >* GetAssets( ) const
			{
				return &mAssetsByName;
			}
			
			/**
			* @brief
			*/
			const Asset* AddToAssets( const String& name, Asset* asset )
			{
				if ( HasAsset( name ) )
				{
					return mAssetsByName[ name ].mAsset;
				} 

				// Set name
				asset->mName = name;

				// Set loader
				asset->mLoader = this;

				// Otherwise add asset
				mAssetsByName[name] = asset;
				mAssetsByUUID[ asset->mUUID.ToString( ) ] = asset;

				return mAssetsByName[name].mAsset;
			} 

		protected:
			
			HashMap< String, AssetRecordInfo > mAssetsByName;
			HashMap< String, AssetRecordInfo > mAssetsByUUID;
			Asset* mDefaultAsset = nullptr;

		private:
			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath, const String& name ) = 0;
	};
} 

#endif
