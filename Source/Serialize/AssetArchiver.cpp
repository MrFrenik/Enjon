// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetArchiver.cpp

#include "Asset/Asset.h"
#include "Asset/AssetLoader.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "Engine.h"

namespace Enjon
{
	//====================================================================================

	Result AssetArchiver::Serialize( const Asset* asset ) 
	{ 
		// Get meta class from asset
		const MetaClass* cls = asset->Class( ); 

		// TODO(): Get versioning structure from meta class this for object...  
		/*
			const MetaClassVersionArchive* archive = cls->GetVersionArchive();
			buffer->Write( archive->GetClassName() );		
			buffer->Write( archive->GetVersionNumber() );
		*/

		// Write out class header information using meta class
		// Class name
		if ( cls )
		{ 
			//==================================================
			// Object Header 
			//==================================================
			mBuffer.Write< String >( cls->GetName( ) );				// Class name
			mBuffer.Write( 0 );										// Version number

			//==================================================
			// Asset Header 
			//==================================================
			mBuffer.Write< UUID >( asset->GetUUID( ) );										// UUID of asset
			mBuffer.Write< String >( asset->GetName( ) );									// Asset name
			mBuffer.Write< String >( asset->GetLoader( )->Class( )->GetName( ) );			// Asset name

			// Serialize all object specific data ( classes can override at this point how they want to serialize data )
			Result res = asset->SerializeData( this );

			// Continue with default serialization if the object doesn't handle its own
			if ( res == Result::INCOMPLETE )
			{
				res = SerializeObjectDataDefault( asset, cls );
			}

			// Final result of serialization
			return res;
		}

		// Shouldn't reach here
		return Result::FAILURE; 
	}

	//====================================================================================

	Result Deserialize( const String& filePath, Vector< Asset* >& out )
	{
		return Result::SUCCESS;
	}

	//====================================================================================

	Asset* AssetArchiver::Deserialize( const String& filePath )
	{ 
		// Reset the buffer
		Reset( );

		// Read contents into buffer
		mBuffer.ReadFromFile( filePath );
 
		//==================================================
		// Object Header 
		//==================================================
		const MetaClass* cls = Object::GetClass( mBuffer.Read< String >( ) );	// Read class type
		u32 versionNumber = mBuffer.Read< u32 >( );								// Read version number id 

		//==================================================
		// Asset Header 
		//==================================================
		UUID uuid = mBuffer.Read< UUID >( );										// UUID of asset
		String assetName = mBuffer.Read< String >( );								// Asset name
		String loaderName = mBuffer.Read< String >( );								// Loader class name

		// Object to construct and fill out
		Asset* asset = nullptr; 

		if ( cls )
		{
			// Construct new object based on class
			asset = (Asset*)cls->Construct( );

			// Couldn't construct object
			if ( !asset )
			{
				delete asset;
				asset = nullptr;
				return nullptr;
			} 
			// Successfully constructed, now deserialize data into it
			else
			{
				Result res = asset->DeserializeData( this ); 

				// Set asset properties
				asset->mLoader = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->GetLoaderByAssetClass( asset->Class( ) );
				asset->mName = assetName;

				// Default deserialization method if not asset does not handle its own deserialization
				if ( res == Result::INCOMPLETE )
				{
					res = DeserializeObjectDataDefault( asset, cls );
				} 

				if ( res != Result::SUCCESS )
				{
					delete asset;
					asset = nullptr;
				} 
			} 
		}

		// Return asset, either null or filled out
		return asset; 
	}
}

