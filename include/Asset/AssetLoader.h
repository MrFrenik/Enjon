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
#include <filesystem>

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

			/**
			* @brief Default Constructor
			*/
			AssetRecordInfo( ) = default;

			/**
			* @brief Constructor
			*/
			AssetRecordInfo( Asset* asset );

			/**
			* @brief Default Destructor
			*/
			~AssetRecordInfo( ) = default;

			/**
			* @brief
			*/
			const Asset* GetAsset( ) const;

			/**
			* @brief
			*/
			String GetAssetName( ) const;

			/**
			* @brief
			*/
			String GetAssetDisplayName( ) const;

			/**
			* @brief
			*/
			AssetLoadStatus GetAssetLoadStatus( ) const; 
			
			/**
			* @brief
			*/
			String GetAssetFilePath( ) const; 

			/**
			* @brief
			*/
			UUID GetAssetUUID( ) const;

			/**
			* @brief
			*/
			void LoadAsset( ) const;

			/**
			* @brief
			*/
			void UnloadAsset( );

			/**
			* @brief
			*/
			void ReloadAsset( );

			/**
			* @brief
			*/
			void Destroy( );


		private:
			Asset* mAsset							= nullptr; 
			String mAssetFilePath					= "Invalid_Asset_Path";
			String mAssetName						= "Invalid_Asset";
			String mAssetDisplayName				= "Invalid_Asset";
			UUID mAssetUUID							= UUID::Invalid( );
			const MetaClass* mAssetLoaderClass		= nullptr;
			AssetLoadStatus mAssetLoadStatus		= AssetLoadStatus::Unloaded;
			AssetLocationType mAssetLocationType	= AssetLocationType::ApplicationAsset;
	}; 

	// Forward declaration
	class AssetManager; 
 
	ENJON_CLASS( )
	class AssetLoader : public Enjon::Object 
	{ 
		friend AssetManager; 
		friend CacheRegistryManifest;
		friend AssetRecordInfo;

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
			bool Exists( const String& name ) const;

			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( )
			{ 
			}

			/**
			* @brief Returns the file extension that this particular asset will use on disk
			*/
			virtual String GetAssetFileExtension( ) const
			{
				return ".easset";
			}

			/**
			* @brief Returns default asset. Will register if not available yet.
			*/
			Asset* GetDefaultAsset( ); 
			
			/**
			* @brief
			*/
			bool Exists( UUID uuid ) const; 

		protected:

			/**
			* @brief
			*/
			void ClearRecords( );

			/**
			* @brief
			*/
			void LoadRecord( AssetRecordInfo* record );

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

			template < typename T >
			Result ConstructAsset( const AssetManager* manager, AssetHandle< T >* handle, const String& assetName = "", const String& path = "" )
			{
				// Make sure that asset with that name doesn't exist already
				if ( Exists( assetName ) )
				{
					handle->Set( GetAsset( assetName ) );
					return Result::SUCCESS;
				}

				// Construct new asset
				T* asset = new T( ); 
				// Copy values from default asset
				*asset = *(T*)GetDefaultAsset( );

				// Construct unique name for asset to be saved
				String typeName = asset->Class( )->GetName( ); 
				String originalAssetName = assetName.compare("") != 0 ? assetName : "New" + typeName;
				String usedAssetName = originalAssetName; 

				// TODO(): MAKE THIS GO THROUGH A CENTRALIZED GRAPHICS FACTORY
				std::experimental::filesystem::path originalPath = manager->GetAssetsDirectoryPath() + "Cache/" + usedAssetName;
				std::experimental::filesystem::path p = originalPath.string() + GetAssetFileExtension();

				// If path is given
				if ( path.compare( "" ) != 0 )
				{
					originalPath = path;
					p = originalPath.string( ) + "/" + usedAssetName + GetAssetFileExtension( );
				}

				// Look for cached asset based on name and continue until name is unique
				u32 index = 0;
				while ( std::experimental::filesystem::exists( p ) )
				{
					index++;
					usedAssetName = originalAssetName + std::to_string( index );
					p = std::experimental::filesystem::path( originalPath.string() + "/" + usedAssetName + GetAssetFileExtension() );
				} 

				String finalAssetName = AssetLoader::GetQualifiedName( p.string() );

				//====================================================================================
				// Asset header information
				//====================================================================================
				AssetRecordInfo info;
				asset->mName = finalAssetName;
				asset->mLoader = this;
				asset->mUUID = UUID::GenerateUUID( ); 
				asset->mFilePath = p.string( );
				asset->mIsDefault = false;

				info.mAsset = asset;
				info.mAssetName = asset->mName;
				info.mAssetUUID = asset->mUUID;
				info.mAssetFilePath = p.string( );					
				info.mAssetLoadStatus = AssetLoadStatus::Loaded;
				info.mAssetLoaderClass = Class( );
				info.mAssetDisplayName = usedAssetName;

				// Add to loader
				const Asset* cnstAsset = AddToAssets( info ); 

				// Set handle with new asset
				handle->Set( cnstAsset );

				// Return incomplete so that manager knows to serialize asset
				return Result::INCOMPLETE;
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
				return &mAssetsByUUID;
			}

			/**
			* @brief
			*/
			const Asset* AddToAssets( const AssetRecordInfo& info ); 

		protected:
			
			HashMap< String, AssetRecordInfo* > mAssetsByName;
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
