// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.cpp

#include "Asset/AssetLoader.h"
#include "Utils/FileUtils.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"

namespace Enjon
{ 
	//=================================================================

	AssetLoader::AssetLoader()
	{ 
	}

	//=================================================================

	AssetLoader::~AssetLoader()
	{
		// Delete all associated assets
		for (auto& asset : mAssetsByName)
		{
			delete asset.second.mAsset;
		}

		mAssetsByName.clear();
		mAssetsByUUID.clear();
	}

	//=================================================================

	String AssetLoader::GetQualifiedName( const String& filePath )
	{
		std::vector<String> splits = Enjon::Utils::SplitString( filePath, "." );
		String res = Enjon::Utils::Remove( Enjon::Utils::Replace( splits.at( 0 ), '/', '.' ), ':' );
		return Enjon::Utils::ToLower( res );
	} 
	
	//=================================================================

	const Asset* AssetLoader::GetAsset( const UUID& id )
	{
		if ( Exists( id ) )
		{
			// Need to check for loaded status here
			AssetRecordInfo* info = &mAssetsByUUID[id.ToString( )];

			// If unloaded, load asset from disk
			if ( info->GetAssetLoadStatus( ) == AssetLoadStatus::Unloaded )
			{
				// Archiver to use to load asset from disk
				AssetArchiver archiver; 

				// Set the asset
				info->mAsset = const_cast<Asset*>( archiver.Deserialize( info->mAssetFilePath )->Cast< Asset >( ) ); 

				// Set loader of asset
				info->mAsset->mLoader = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->GetLoaderByAssetClass( info->mAsset->Class( ) );

				// Set to loaded
				info->mAssetLoadStatus = AssetLoadStatus::Loaded; 
			} 

			// Return asset from info record
			if ( !info->mAsset )
			{
				return GetDefaultAsset( );
			}

			return info->mAsset;
		}

		return GetDefaultAsset( ); 
	} 
	
	//=================================================================

	const Asset* AssetLoader::GetAsset( const String& name )
	{
		if ( Exists( name ) )
		{
			// Need to check for loaded status here
			AssetRecordInfo* info = &mAssetsByName[name];

			// If unloaded, load asset from disk
			if ( info->GetAssetLoadStatus( ) == AssetLoadStatus::Unloaded )
			{
				// Archiver to use to load asset from disk
				AssetArchiver archiver; 

				// Set the asset
				info->mAsset = const_cast<Asset*>( archiver.Deserialize( info->mAssetFilePath )->Cast< Asset >( ) );

				// Set loader of asset
				info->mAsset->mLoader = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->GetLoaderByAssetClass( info->mAsset->Class( ) );

				// Set to loaded
				info->mAssetLoadStatus = AssetLoadStatus::Loaded;

				// Set up asset info
				info->mAsset->mName = info->mAssetName; 
			}

			// Return asset from info record
			if ( !info->mAsset )
			{
				return GetDefaultAsset( );
			}

			return info->mAsset;
		}

		return GetDefaultAsset( ); 
	}
	
	//=================================================================

	bool AssetLoader::FindRecordInfoByName( const String& name, AssetRecordInfo* info )
	{
		for ( auto iter = mAssetsByUUID.begin( ); iter != mAssetsByUUID.end( ); ++iter )
		{
			if ( iter->second.GetAssetName( ) == name )
			{
				*info = iter->second;
				return true;
			}
		}

		return false;
	}
	
	//=================================================================
			
	bool AssetLoader::Exists( const String& name ) const
	{
		auto query = mAssetsByName.find( name );
		if ( query != mAssetsByName.end() )
		{
			return true;
		}

		return false;
	}
	
	//=================================================================
			
	bool AssetLoader::Exists( UUID uuid ) const
	{
		auto query = mAssetsByUUID.find( uuid.ToString() );
		if ( query != mAssetsByUUID.end( ) )
		{
			return true;
		}

		return false;
	} 
	
	//=================================================================

	Asset* AssetLoader::GetDefaultAsset( )
	{
		if ( mDefaultAsset )
		{
			return mDefaultAsset;
		}

		RegisterDefaultAsset( ); 
		return mDefaultAsset;
	} 
	
	//=================================================================

	const Asset* AssetLoader::AddToAssets( const AssetRecordInfo& info )
	{ 
		// If the UUID exists for whatever reason, return that asset
		if ( info.mAsset->mUUID && Exists( info.mAsset->GetUUID() ) )
		{
			return mAssetsByUUID[ info.mAsset->GetUUID( ).ToString( ) ].mAsset;
		} 

		// Add asset
		mAssetsByName[ info.mAsset->GetName() ] = info;
		mAssetsByUUID[ info.mAsset->GetUUID( ).ToString( ) ] = info;

		return info.mAsset; 
	}
	
	//=================================================================

	Result AssetLoader::AddRecord( const CacheManifestRecord& record )
	{
		// Info record to fill out based on manifest record
		AssetRecordInfo info; 
		info.mAssetLoadStatus = AssetLoadStatus::Unloaded;
		info.mAssetFilePath = record.mAssetFilePath;
		info.mAssetName = record.mAssetName;
		info.mAssetUUID = record.mAssetUUID;
		info.mAsset = nullptr;

		// Add to assets
		if ( !Exists( record.mAssetUUID ) )
		{
			mAssetsByUUID[ record.mAssetUUID.ToString( ) ] = info;

			// Don't know if I like this or not...
			mAssetsByName[ record.mAssetName ] = info;

			return Result::SUCCESS;
		}

		return Result::FAILURE;
	} 
	
	//=================================================================
}

