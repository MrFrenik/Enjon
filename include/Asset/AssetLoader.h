// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.h

#pragma once
#ifndef ENJON_ASSET_LOADER_H
#define ENJON_ASSET_LOADER_H 

 #include "Asset/Asset.h" 
#include "Serialize/CacheRegistryManifest.h"
#include "Defines.h"

#include <string>
#include <unordered_map>

namespace Enjon
{
	ENJON_ENUM( )
	enum class AssetLoadStatus
	{
		Unloaded,				// Not loaded in to memory but record is created
		Loaded					// Fully loaded into memory and ready to use
	};

	class AssetLoader;
	class AssetRecordInfo
	{
		friend AssetLoader;
		friend AssetManager;

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

			AssetLoadStatus GetAssetLoadStatus( ) const
			{
				return mAssetLoadStatus;
			} 

		private:
			Asset* mAsset						= nullptr; 
			String mAssetFilePath				= "";
			String mAssetName					= "";
			UUID mAssetUUID						= UUID::Invalid( );
			AssetLoadStatus mAssetLoadStatus	= AssetLoadStatus::Unloaded;
	}; 

	// Forward declaration
	class AssetManager; 
 
	ENJON_CLASS( )
	class AssetLoader : public Enjon::Object 
	{ 
		friend AssetManager; 
		friend CacheRegistryManifest;

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
			* @brief Templated argument to get asset of specific type by name
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
			const Asset* GetAsset( const String& name );

			/**
			* @brief Adds an unloaded AssetRecordInfo struct with all the necessary information for loading the asset during runtime.
			*			When the asset is to be loaded, this struct is searched for ( either by UUID or name ) and the asset is then deserialized off disk.
			*			If during the deserialization process the asset is not found, the default asset for this particular asset type is returned.
			*/
			Result AddRecord( const CacheManifestRecord& record );

			/**
			* @brief 
			*/
			const Asset* GetAsset( const UUID& id );
			
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
				// Already exists
				if ( HasAsset( name ) )
				{
					return mAssetsByName[ name ].mAsset;
				} 

				// Set name
				asset->mName = name;

				// Set loader
				asset->mLoader = this;

				// Otherwise add asset
				mAssetsByName[name] = AssetRecordInfo( asset );
				mAssetsByUUID[ asset->mUUID.ToString( ) ] = AssetRecordInfo( asset );

				return mAssetsByName[name].mAsset;
			} 

			/**
			* @brief
			*/
			const Asset* AddToAssets( const AssetRecordInfo& info );

		protected:
			
			HashMap< String, AssetRecordInfo > mAssetsByName;
			HashMap< String, AssetRecordInfo > mAssetsByUUID;
			Asset* mDefaultAsset = nullptr;

		private: 

			/**
			* @brief 
			*/ 
			bool FindRecordInfoByName( const String& name, AssetRecordInfo* info );

			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromFile( const String& filePath ) = 0;
	};
} 

#endif
