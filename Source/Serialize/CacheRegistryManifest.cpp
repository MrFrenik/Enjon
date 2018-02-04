// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: CacheRegistryManifest.cpp 

#include "Serialize/CacheRegistryManifest.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"

#include <filesystem>

namespace Enjon
{ 
	//=========================================================================================

	void CacheRegistryManifest::Reset()
	{
		// Reset path 
		mManifestPath = "";
		// Clear records
		mManifestRecords.clear();
	}

	//=========================================================================================

	Result CacheRegistryManifest::Initialize( const String& manifestPath, const AssetManager* manager )
	{
		// Check if directory exists - if not, create it
		if ( !std::experimental::filesystem::exists( manager->GetCachedAssetsDirectoryPath( ) ) )
		{
			std::experimental::filesystem::create_directory( manager->GetCachedAssetsDirectoryPath( ) );
		} 

		// Reset manifest records
		Reset();

		// Set manifest path
		mManifestPath = manifestPath;
		// Set asset manager
		mAssetManager = manager;

		// Open a binary file to parse and do stuff with it
		Result res = ReadInManifest( );

		return res;
	}

	//=========================================================================================

	Result CacheRegistryManifest::WriteOutManifest( const String& manifestPath  )
	{
		// Create write buffer
		ByteBuffer buffer;

		// Amount of records to store
		buffer.Write< u32 >( mManifestRecords.size( ) );

		// For each record, write out information into buffer
		for ( auto iter = mManifestRecords.begin( ); iter != mManifestRecords.end( ); ++iter )
		{
			// Write out UUID of record
			buffer.Write< UUID >( iter->second.mAssetUUID );
			// Write out file path of record
			buffer.Write< String >( iter->second.mAssetFilePath );
			// Write out name of asset
			buffer.Write< String >( iter->second.mAssetName );
			// Write out MetaClass name of loader
			buffer.Write< String >( iter->second.mAssetLoaderClass != nullptr ? iter->second.mAssetLoaderClass->GetName() : "" );
		}

		// Write to file using manifest path
		buffer.WriteToFile( manifestPath );

		return Result::SUCCESS;
	}

	//=========================================================================================

	Result CacheRegistryManifest::ReadInManifest( )
	{
		for ( auto& p : std::experimental::filesystem::recursive_directory_iterator( mAssetManager->GetAssetsDirectoryPath( ) + "/" ) )
		{
			if ( Enjon::AssetManager::HasFileExtension( p.path( ).string( ), "easset" ) )
			{
				// Asset record to fill out
				CacheManifestRecord record; 

				// Buffer to read from
				ByteBuffer buffer( p.path( ).string( ) ); 

				//==================================================
				// Object Header 
				//==================================================
				const MetaClass* cls = Object::GetClass( buffer.Read< String >( ) );	// Read class type
				u32 versionNumber = buffer.Read< u32 >( );								// Read version number id 

				//==================================================
				// Asset Header 
				//================================================== 
				record.mAssetUUID = buffer.Read< UUID >();									// UUID of asset
				record.mAssetName = buffer.Read< String >();								// Asset name
				record.mAssetLoaderClass = Object::GetClass( buffer.Read< String >( ) );	// Loader class
				record.mAssetFilePath = p.path( ).string( );								// Asset file path 

				// Set location type of record
				record.mAssetLocationType = mAssetManager->GetAssetLocationType( );

				// Add record
				AddRecord( record );

				if ( mAssetManager )
				{
					// At this point, should I pass on the record to the loader class? 
					const AssetLoader* loader = mAssetManager->GetLoader( record.mAssetLoaderClass );

					if ( loader )
					{
						// Add record to loader
						const_cast<AssetLoader*>( loader )->AddRecord( record );
					}
				} 
			} 
		}

		return Result::SUCCESS;

		//// Create read buffer from manifest path
		//ByteBuffer buffer( mManifestPath );

		//// If the buffer was created successfully, then parse
		//if ( buffer.GetStatus( ) == BufferStatus::ReadyToRead )
		//{
		//	// Grab record count
		//	u32 recordCount = buffer.Read< u32 >();

		//	// For each record, store into manifest 
		//	for ( u32 i = 0; i < recordCount; ++i )
		//	{
		//		// Asset record to fill out
		//		CacheManifestRecord record; 
		//		
		//		// Load into record
		//		record.mAssetUUID = buffer.Read< UUID >();
		//		record.mAssetFilePath = buffer.Read< String >();
		//		record.mAssetName = buffer.Read< String >( );

		//		// Get MetaClass for loader
		//		const MetaClass* cls = Object::GetClass( buffer.Read< String >() );
		//		record.mAssetLoaderClass = cls;

		//		// Add record
		//		AddRecord( record );

		//		if ( mAssetManager )
		//		{
		//			// At this point, should I pass on the record to the loader class? 
		//			const AssetLoader* loader = mAssetManager->GetLoader( cls );

		//			if ( loader )
		//			{
		//				// Add record to loader
		//				const_cast< AssetLoader* >( loader )->AddRecord( record ); 
		//			} 
		//		}
		//	} 

		//	return Result::SUCCESS;
		//} 

		//return Result::FAILURE; 
	}

	//=========================================================================================

	bool CacheRegistryManifest::HasRecord( const UUID& uuid )
	{
		return ( mManifestRecords.find( uuid.ToString() ) != mManifestRecords.end( ) );
	}

	//=========================================================================================

	Result CacheRegistryManifest::AddRecord( const CacheManifestRecord& record )
	{
		if ( !HasRecord( record.mAssetUUID ) )
		{
			mManifestRecords[record.mAssetUUID.ToString()] = record;

			return Result::SUCCESS;
		}

		return Result::FAILURE;
	}
}

