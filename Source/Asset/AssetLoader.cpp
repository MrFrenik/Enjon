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
		// If asset isn't available, then load it from disk
		if ( !mAsset )
		{
			LoadAsset( );
		}

		return mAsset;
	}

	//=================================================================

	void AssetRecordInfo::LoadAsset( ) const
	{
		// Get the asset manager from engine
		AssetManager* am = EngineSubsystem( AssetManager );

		// Grab the loader
		AssetLoader* al = am->GetLoader( mAssetLoaderClass )->ConstCast< AssetLoader >( );

		// Get asset from asset loader
		const Asset* asset = al->GetAsset( mAssetUUID );

		// Set the asset
		const_cast<AssetRecordInfo *> ( this )->mAsset = const_cast<Asset*>( asset );
	}

	//=================================================================

	void AssetRecordInfo::UnloadAsset( )
	{
		// Delete the asset if not null
		if ( mAsset )
		{
			// Delete asset
			delete mAsset;

			// Set to null for future
			mAsset = nullptr;
		}

		// Set load status to unloaded after releasing memory
		mAssetLoadStatus = AssetLoadStatus::Unloaded;
	}

	//=================================================================

	void AssetRecordInfo::ReloadAsset( )
	{
		// Do not want to unload asset, since other assethandle references will be lost
		if ( mAsset )
		{
			// Archiver to use to load asset from disk
			AssetArchiver archiver;

			ByteBuffer buffer;
			buffer.ReadFromFile( mAssetFilePath );

			// Error checking...
			if ( buffer.GetStatus( ) == BufferStatus::Invalid )
			{
				return;
			}

			// Set the asset
			AssetArchiver::Deserialize( &buffer, mAsset );
		}
	}

	//=================================================================

	void AssetRecordInfo::Destroy( )
	{
		UnloadAsset( );
	}

	//=================================================================

	String AssetRecordInfo::GetAssetName( ) const
	{
		return mAssetName;
	}

	//=================================================================

	String AssetRecordInfo::GetAssetDisplayName( ) const
	{
		return mAssetDisplayName;
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

	UUID AssetRecordInfo::GetAssetUUID( ) const
	{
		return mAssetUUID;
	} 

	//=================================================================

	void AssetLoader::ExplicitDestructor( )
	{
		// Delete all associated assets
		for ( auto& asset : mAssetsByUUID )
		{
			delete asset.second.mAsset;
		}

		mAssetsByName.clear( );
		mAssetsByUUID.clear( );

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
		String finalPath = Utils::FindReplaceAll( filePath, "\\", "/" );
		finalPath = Utils::FindReplaceAll( finalPath, "//", "/" );
		Vector<String> splits = Utils::SplitString( finalPath, "." );
		String res = Utils::Remove( Enjon::Utils::Replace( splits.size( ) > 1 ? splits.at( splits.size( ) - 2 ) : splits.at( 0 ), '/', '.' ), ':' );
		res = Utils::FindReplaceAll( res, "\\", "." );
		Vector<String> sp2 = Utils::SplitString( res, ".Assets." );
		String finalRes = sp2.size( ) > 1 ? sp2.at( sp2.size( ) - 1 ) : res;
		Vector<String> sp3 = Utils::SplitString( finalRes, "." );
		u32 startIdx = sp3.at( 0 ).compare( "Assets" ) == 0 ? 1 : 0;
		finalRes = "";
		for ( usize i = startIdx; i < sp3.size( ); ++i )
		{
			finalRes += sp3.at( i );
			finalRes += i == sp3.size( ) - 1 ? "" : ".";
		}
		return Enjon::Utils::ToLower( finalRes );
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

				// Set asset file path
				info->mAsset->mFilePath = info->mAssetFilePath;
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
		String qualifiedName = Utils::ToLower( name );

		if ( Exists( qualifiedName ) )
		{
			// Need to check for loaded status here
			AssetRecordInfo* info = mAssetsByName[qualifiedName];

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

				// Set asset file path
				info->mAsset->mFilePath = info->mAssetFilePath;
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
		String qualifiedName = Utils::ToLower( name );

		for ( auto iter = mAssetsByUUID.begin( ); iter != mAssetsByUUID.end( ); ++iter )
		{
			if ( iter->second.GetAssetName( ).compare( qualifiedName ) == 0 )
			{
				*info = iter->second;
				return true;
			}
		}

		return false;
	}

	//=================================================================

	Asset* AssetLoader::LoadResourceFromImporter( const ImportOptions* options )
	{
		return nullptr;
	}

	//=================================================================

	Asset* AssetLoader::LoadResourceFromFile( const String& filePath )
	{
		return nullptr;
	}

	//=================================================================

	Asset* AssetLoader::DirectImport( const ImportOptions* options )
	{
		return nullptr;
	}

	//=================================================================

	bool AssetLoader::Exists( const String& name ) const
	{
		auto query = mAssetsByName.find( name );
		if ( query != mAssetsByName.end( ) )
		{
			return true;
		}

		return false;
	}

	//=================================================================

	bool AssetLoader::Exists( UUID uuid ) const
	{
		auto query = mAssetsByUUID.find( uuid.ToString( ) );
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

		// Set default asset to being default
		mDefaultAsset->mIsDefault = true;

		return mDefaultAsset;
	}

	//=================================================================

	const Asset* AssetLoader::AddToAssets( const AssetRecordInfo& info )
	{
		// If the UUID exists for whatever reason, return that asset
		if ( info.mAsset->mUUID && Exists( info.mAsset->GetUUID( ) ) )
		{
			return mAssetsByUUID[info.mAsset->GetUUID( ).ToString( )].mAsset;
		}

		// Add asset
		mAssetsByUUID[info.mAsset->GetUUID( ).ToString( )] = info;
		mAssetsByName[info.mAsset->GetName( )] = &mAssetsByUUID[info.mAsset->GetUUID( ).ToString( )];

		// Set info for asset
		info.mAsset->mRecordInfo = &mAssetsByUUID[info.mAsset->GetUUID( ).ToString( )];

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
			mAssetsByUUID[record.mAssetUUID.ToString( )] = info;
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
			mAssetsByUUID[rec.mAssetUUID.ToString( )] = rec;
			mAssetsByName[rec.mAssetName] = &mAssetsByUUID[rec.mAssetUUID.ToString( )];
		}
	}

	//=================================================================

	void AssetLoader::RenameAssetFilePath( Asset* asset, const String& path )
	{
		// Grab the record info for this asset
		AssetRecordInfo* info = &mAssetsByUUID[asset->GetUUID( ).ToString( )];

		// Set info file path
		info->mAssetFilePath = path;

		// Have to change a lot here...
		info->mAsset->mFilePath = path;

		// Remove from name assets map
		mAssetsByName.erase( info->mAssetName );

		// Get new qualified asset name
		String newAssetName = AssetLoader::GetQualifiedName( path );

		// Set info asset name
		info->mAssetName = newAssetName;

		// Set raw asset name
		info->mAsset->mName = newAssetName;

		// Put into map
		mAssetsByName[info->mAssetName] = info;
	}

	//=================================================================

	bool AssetLoader::IsImporting( ) const
	{
		if ( GetImportOptions() )
		{
			return GetImportOptions()->IsImporting( );
		}

		return false;
	}

	//=================================================================

	void AssetLoader::BeginImport( const String& filePath, const String& cacheDirectory )
	{ 
		if ( GetImportOptions( ) )
		{
			// Set import options to importing
			GetImportOptions( )->ConstCast< ImportOptions >()->mIsImporting = true;

			// Begin internal import process
			Result res = BeginImportInternal( filePath, cacheDirectory );
		} 
		else
		{
			// Basic import at this process
			EngineSubsystem( AssetManager )->AddToDatabase( filePath, cacheDirectory ); 
		}
	} 

	//=================================================================

	Result AssetLoader::BeginImportInternal( const String& filePath, const String& cacheDirectory )
	{
		// Nothing by default, so reset...	
		if ( GetImportOptions( ) )
		{
			// Set import options to importing
			GetImportOptions( )->ConstCast< ImportOptions >()->mIsImporting = false; 
		}

		return Result::SUCCESS;
	}

	//=================================================================

	const ImportOptions* AssetLoader::GetImportOptions( ) const
	{
		// Nothing by default
		return nullptr;
	} 

	//=================================================================
}

