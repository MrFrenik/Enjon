// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.cpp

#include "Asset/AssetLoader.h"
#include "Utils/FileUtils.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{ 
	//=================================================================
 
	AssetRecordInfo::AssetRecordInfo( Asset* asset )
		: mAsset( asset )
	{
	} 

	//=================================================================

	const Asset* AssetRecordInfo::GetAsset( ) const
	{
		if ( !mAsset )
		{
			// Get the asset manager from engine
			const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );

			// Grab the loader
			AssetLoader* al = am->GetLoader( mAssetLoaderClass )->ConstCast< AssetLoader >();

			//Asset* asset = al->LoadAssetForRecord( mAssetUUID ); 
			const Asset* asset = al->GetAsset( mAssetUUID ); 
			
			const_cast< AssetRecordInfo *> ( this )->mAsset = const_cast< Asset* >( asset );
		}

		return mAsset;
	} 

	//=================================================================

	void AssetRecordInfo::Destroy( )
	{
		// Release memory for asset
		delete mAsset;

		// Set to nullptr
		mAsset = nullptr;
	}

	//=================================================================

	String AssetRecordInfo::GetAssetName( ) const
	{
		return mAssetName;
	}

	//=================================================================

	AssetLoadStatus AssetRecordInfo::GetAssetLoadStatus( ) const
	{
		return mAssetLoadStatus;
	} 

	//================================================================= 
	
	String AssetRecordInfo::GetAssetFilePath( ) const 
	{
		return mAssetFilePath;
	}

	//=================================================================

	AssetLoader::AssetLoader()
	{ 
	}

	//=================================================================

	AssetLoader::~AssetLoader()
	{
		// Delete all associated assets
		for (auto& asset : mAssetsByUUID)
		{
			delete asset.second.mAsset;
		}

		mAssetsByName.clear();
		mAssetsByUUID.clear();

		// Delete default asset
		if ( mDefaultAsset )
		{
			delete mDefaultAsset; 
			mDefaultAsset = nullptr;
		} 
	}

	//=================================================================

	String AssetLoader::GetQualifiedName( const String& filePath )
	{
		std::vector<String> splits = Enjon::Utils::SplitString( filePath, "." );
		String res = Enjon::Utils::Remove( Enjon::Utils::Replace( splits.at( 0 ), '/', '.' ), ':' );
		return Enjon::Utils::ToLower( res );
	} 
	
	//=================================================================

	void AssetLoader::LoadRecord( AssetRecordInfo* info )
	{
		// Load asset from record and set
		const Asset* asset = GetAsset( info->mAssetUUID ); 
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

				// Set record info of asset
				info->mAsset->mRecordInfo = info; 
			} 

			// Set default asset if not valid asset
			if ( !info->mAsset )
			{
				info->mAsset = GetDefaultAsset( );
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
			AssetRecordInfo* info = mAssetsByName[name];

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

				// Set record info for asset
				info->mAsset->mRecordInfo = info; 
			}

			// Return asset from info record
			if ( !info->mAsset )
			{
				info->mAsset = GetDefaultAsset( );
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
		mAssetsByUUID[ info.mAsset->GetUUID( ).ToString( ) ] = info;
		mAssetsByName[ info.mAsset->GetName() ] = &mAssetsByUUID[ info.mAsset->GetUUID().ToString() ];

		return info.mAsset; 
	}
	
	//=================================================================

	Result AssetLoader::AddRecord( const CacheManifestRecord& record )
	{
		// Info record to fill out based on manifest record
		AssetRecordInfo info; 
		info.mAssetLoadStatus = AssetLoadStatus::Unloaded;
		info.mAssetLocationType = record.mAssetLocationType;
		info.mAssetFilePath = record.mAssetFilePath;
		info.mAssetName = record.mAssetName;
		info.mAssetUUID = record.mAssetUUID;
		info.mAssetLoaderClass = record.mAssetLoaderClass;
		info.mAsset = nullptr;

		// Add to assets
		if ( !Exists( record.mAssetUUID ) )
		{
			// Store record by UUID
			mAssetsByUUID[ record.mAssetUUID.ToString( ) ] = info; 
			// Store pointer to record by asset name
			mAssetsByName[record.mAssetName] = &mAssetsByUUID[record.mAssetUUID.ToString( )];

			return Result::SUCCESS;
		}

		return Result::FAILURE;
	} 
	
	//=================================================================

	void AssetLoader::ClearRecords( )
	{
		Vector< AssetRecordInfo > mEngineAssetRecords;

		// Clear all assets by uuid
		for ( auto& a : mAssetsByUUID )
		{
			if ( a.second.mAssetLocationType == AssetLocationType::EngineAsset )
			{
				mEngineAssetRecords.push_back( a.second );
			}
			else
			{
				a.second.Destroy( ); 
			}
		}

		mAssetsByName.clear( );
		mAssetsByUUID.clear( );

		// Reassign engine asset records
		for ( auto& rec : mEngineAssetRecords )
		{
			mAssetsByUUID[ rec.mAssetUUID.ToString( ) ] = rec;
			mAssetsByName[ rec.mAssetName ] = &mAssetsByUUID[ rec.mAssetUUID.ToString() ];
		}
	}
	
	//=================================================================
}

