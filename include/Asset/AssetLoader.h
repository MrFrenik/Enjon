// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.h

#pragma once
#ifndef ENJON_ASSET_LOADER_H
#define ENJON_ASSET_LOADER_H 

#include "Asset/ImportOptions.h"
#include "Asset/Asset.h" 
#include "Serialize/CacheRegistryManifest.h"
#include "Defines.h"

#include <string>
#include <unordered_map>
#include "fs/filesystem.hpp"

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

			/**
			* @brief
			*/
			const MetaClass* GetAssetClass( );


		private:
			Asset* mAsset							= nullptr; 
			String mAssetFilePath					= "Invalid_Asset_Path";
			String mAssetName						= "Invalid_Asset";
			String mAssetDisplayName				= "Invalid_Asset";
			UUID mAssetUUID							= UUID::Invalid( );
			const MetaClass* mAssetLoaderClass		= nullptr;
			AssetLoadStatus mAssetLoadStatus		= AssetLoadStatus::Unloaded;
			AssetLocationType mAssetLocationType	= AssetLocationType::ApplicationAsset;
			const MetaClass* mAssetClass			= nullptr;
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
		ENJON_CLASS_BODY( AssetLoader )
		
		public: 

			/**
			* @brief Explicit Destructor
			*/
			virtual void ExplicitDestructor( ) override;

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
			void ReloadAsset( Asset* asset );

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

			/**
			* @brief
			*/
			bool IsImporting( ) const;

			/**
			* @brief Construct import options to use
			*/
			void BeginImport( const String& filepath, const String& cacheDirectory );

			/**
			* @brief 
			*/
			virtual const ImportOptions* GetImportOptions( ) const;

			/**
			* @brief DO NOT CALL DIRECTLY
			*/
			virtual Asset* DirectImport( const ImportOptions* options );

			/**
			* @brief DO NOT CALL DIRECTLY
			*/
			void UnloadAssets( );

		protected: 

			/**
			* @brief
			*/
			virtual Result BeginImportInternal( const String& filepath, const String& cacheDirectory );

			/**
			* @brief
			*/
			void ClearRecords( );

			/**
			* @brief
			*/
			void LoadRecord( AssetRecordInfo* record );

			/**
			* @brief
			*/
			void RenameAssetFilePath( Asset* asset, const String& path ); 

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
					handle = AssetHandle<T>( query->second->mAsset ); 
				}
				// Else return default asset
				else
				{
					handle = AssetHandle<T>( GetDefault( ) );
				}

				return handle; 
			} 

			/*
			* @brief Implementation in AssetManager.h
			*/
			template < typename T >
			Result ConstructAsset( const AssetManager* manager, AssetHandle< T >* handle, const String& assetName = "", const String& path = "" );

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
			virtual Asset* LoadResourceFromFile( const String& filePath ); 

			/**
			* @brief 
			*/
			virtual Asset* LoadResourceFromImporter( const ImportOptions* options ); 
	};

	//====================================================================================
	//====================================================================================

	//===============
	// Asset Handle
	//===============

	template < typename T >
	void AssetHandle< T >::Unload( )
	{
		// Unload asset from record info if valid
		if ( mAsset )
		{
			const_cast< AssetRecordInfo* >( mAsset->GetAssetRecordInfo( ) )->UnloadAsset( );
		}
	}

	template < typename T >
	void AssetHandle< T >::Reload( )
	{
		if ( mAsset )
		{
			// Reload and reset asset
			const_cast<AssetRecordInfo*>( mAsset->GetAssetRecordInfo( ) )->ReloadAsset( );
		}
	}
} 

#endif
