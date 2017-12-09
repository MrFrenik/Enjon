// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.cpp

#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "Asset/TextureAssetLoader.h" 
#include "Asset/MeshAssetLoader.h" 
#include "Asset/FontAssetLoader.h"
#include "Asset/MaterialAssetLoader.h"
#include "Asset/ShaderGraphAssetLoader.h"
#include "Utils/FileUtils.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/AssetArchiver.h"
#include "Engine.h"

#include <fmt/printf.h>

namespace Enjon
{
	//============================================================================================ 

	AssetManager::AssetManager() 
	{
		// Set to default
		mName = "";

		// Set to default
		mAssetsPath = "";
		
		// Register the loaders with manager 
		RegisterLoaders( );
	}

	//============================================================================================ 

	AssetManager::AssetManager(const String& name, const String& assetsPath)
	{
		// Set name
		mName = name;

		// Set project asset path
		mAssetsPath = assetsPath; 

		// Register the loaders with manager 
		RegisterLoaders( );
	}
	
	//============================================================================================ 
			
	AssetManager::~AssetManager()
	{ 
		// Release all loaders from memory
		for (auto& loader : mLoadersByAssetId)
		{
			delete loader.second;
		}

		// Clear map
		mLoadersByAssetId.clear();
	}
	
	//============================================================================================ 
			
	void AssetManager::RegisterLoaders( )
	{
		// Register the loaders with manager 
		RegisterAssetLoader< Texture,		TextureAssetLoader >( );
		RegisterAssetLoader< Mesh,			MeshAssetLoader >( );
		RegisterAssetLoader< UIFont,		FontAssetLoader >( );
		RegisterAssetLoader< ShaderGraph,	ShaderGraphAssetLoader >( );
		RegisterAssetLoader< Material,		MaterialAssetLoader >( );

		// Create file extension map
		mFileExtensionMap[ "png" ]	= GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "tga" ]	= GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "jpeg" ] = GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "bmp" ]	= GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "hdr" ]	= GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "fbx" ]	= GetAssetTypeId< Mesh >( );
		mFileExtensionMap[ "obj" ]	= GetAssetTypeId< Mesh >( );
		mFileExtensionMap[ "ttf" ]	= GetAssetTypeId< UIFont >( );
		mFileExtensionMap[ "otf" ]	= GetAssetTypeId< UIFont >( ); 
		mFileExtensionMap[ "sg" ]	= GetAssetTypeId< ShaderGraph >( ); 
	}
	
	//============================================================================================ 
			
	Result AssetManager::Initialize()
	{ 
		// Initialize the manifest and read in records
		mCacheManifest.Initialize( mAssetsPath + "/Intermediate/CacheManifest.bin", this ); 

		return Result::SUCCESS;
	} 
	
	//============================================================================================ 

	void AssetManager::Update( const f32 dT )
	{ 
		// In here could check for file updates on hot loaded resources
	}
	
	//============================================================================================ 

	Result AssetManager::Shutdown()
	{
		return Result::SUCCESS;
	}
	
	//============================================================================================ 
			
	void AssetManager::SetAssetsPath( const String& path )
	{
		mAssetsPath = path;
	}
	
	//============================================================================================ 
			
	Enjon::String AssetManager::GetAssetsPath( ) const
	{
		return mAssetsPath;
	}
	
	//============================================================================================ 
			
	void AssetManager::SetCachedAssetsPath( const String& filePath )
	{
		mCachedPath = filePath;
	}
	
	//============================================================================================ 
	
	const Enjon::String& AssetManager::GetCachedAssetsPath( ) const
	{
		return mCachedPath;
	}
	
	//============================================================================================ 
			
	void AssetManager::SetDatabaseName( const String& name )
	{
		mName = name;
	}
	
	//============================================================================================ 

	s32 AssetManager::GetLoaderIdxByFileExtension( const String& filePath )
	{ 
		// If not found, will return -1
		s32 idx = -1;

		// Get file extension of file
		String fileExtension = Utils::SplitString( filePath, "." ).back( ); 

		// Search for file extension relation with loader
		auto query = mFileExtensionMap.find( fileExtension );
		if ( query != mFileExtensionMap.end( ) )
		{
			idx = mFileExtensionMap[ fileExtension ];
		}

		return idx; 
	}
	
	//============================================================================================ 
			
	Enjon::String AssetManager::GetName( )
	{
		return mName;
	} 

	//============================================================================================ 

	Asset* AssetManager::GetDefaultAsset( const Enjon::MetaClass* cls )
	{
		// Make sure class is valid
		assert( cls != nullptr );

		// Get type id of class
		u32 idx = cls->GetTypeId( ); 

		if ( Exists( idx ) )
		{
			return mLoadersByAssetId[ idx ]->GetDefault( );
		}

		return nullptr;
	}
	
	//============================================================================================ 
			
	const Asset* AssetManager::GetAsset( const MetaClass* cls, const UUID& id )
	{
		// Get type id of class
		u32 idx = cls->GetTypeId( );

		if ( Exists( idx ) )
		{
			return mLoadersByAssetId[ idx ]->GetAsset( id );
		}

		return nullptr;
	}
	
	//============================================================================================ 
			
	const HashMap< Enjon::String, AssetRecordInfo >* AssetManager::GetAssets( const Enjon::MetaClass* cls )
	{
		// Make sure class is valid
		assert( cls != nullptr );

		// Get type id of class
		u32 idx = cls->GetTypeId( );

		if ( Exists( idx ) )
		{
			return mLoadersByAssetId[ idx ]->GetAssets( );
		} 

		return nullptr;
	}

	/*
		How to create a new material asset and then serialize it?
	 
		// Needs a unique name:
		//	- Save out default name as "New" + cls->GetName() + unique instance number // For instance, a new material will have name "NewMaterial1", if there was one other existing file named "NewMaterial"
		AssetHandle<Asset> AssetManager::CreateAsset( const MetaClass* cls )
		{
					
		}

	*/ 
 
	//============================================================================================ 
			
	Result AssetManager::AddToDatabase( const String& resourceFilePath, bool cache, bool isRelativePath )
	{ 
		// To be determined, whether or not the asset will need to be cached
		bool needToCache = false;

		// Have to do a switch based on extension of file
		s32 idx = GetLoaderIdxByFileExtension( resourceFilePath );

		// Asset to be returned from loading if successful
		Asset* asset = nullptr;

		// If out of bounds, return failure since file extension was unknown
		if ( idx < 0 )
		{
			// TODO(): Log that extension was unknown
			return Result::FAILURE;
		}

		// Get qualified name of asset
		String qualifiedName = AssetLoader::GetQualifiedName( resourceFilePath );

		// Find loader by idx
		auto query = mLoadersByAssetId.find( ( u32 )idx );
		if ( query != mLoadersByAssetId.end( ) )
		{
			if ( !query->second )
			{
				return Result::FAILURE;
			}

			// Make sure it doesn't exist already before trying to load it
			if ( query->second->Exists( qualifiedName ) )
			{
				return Result::FAILURE;
			}
			else
			{
				// Load asset and place into database
				if ( isRelativePath )
				{
					// Return failure if path doesn't exist
					if ( !Utils::FileExists( mAssetsPath + resourceFilePath ) )
					{
						return Result::FAILURE;
					} 

					// If file exists, grab that
					if ( query->second->Exists( Utils::ToLower( mName ) + qualifiedName ) )
					{
						asset = const_cast< Asset* >( query->second->GetAsset( Utils::ToLower( mName ) + qualifiedName ) );
					}

					else
					{
						// We need to cache the asset at the end of this operation
						needToCache = true;

						// Load the asset from file
						asset = query->second->LoadResourceFromFile( mAssetsPath + resourceFilePath );
	 
						// If asset is valid
						if ( asset )
						{
							// Add to loader assets with asset record info
							AssetRecordInfo info;
							asset->mName = Utils::ToLower( mName ) + qualifiedName;
							asset->mFilePath = mAssetsPath + resourceFilePath;				// THIS IS INCORRECT! NEED TO CHANGE TO BEING THE ACTUAL CACHED ASSET PATH!
							asset->mUUID = UUID::GenerateUUID( );
							asset->mLoader = query->second;
							info.mAsset = asset;
							info.mAssetName = asset->mName;
							info.mAssetUUID = asset->mUUID;
							info.mAssetFilePath = asset->mFilePath;							// THIS IS INCORRECT! NEED TO CHANGE TO BEING THE ACTUAL CACHED ASSET PATH!
							info.mAssetLoadStatus = AssetLoadStatus::Loaded;

							// Add to loader
							query->second->AddToAssets( info );
						} 
					} 
				}

				// If absolute path on disk
				else
				{
					// Return failure if path doesn't exist
					if ( !Utils::FileExists( resourceFilePath ) )
					{
						return Result::FAILURE;
					}

					// Load asset from file
					asset = query->second->LoadResourceFromFile( resourceFilePath );

					// If asset is valid
					if ( asset )
					{
						// Add to loader assets with asset record info
						AssetRecordInfo info;
						asset->mName = qualifiedName;
						asset->mFilePath = resourceFilePath;
						asset->mUUID = UUID::GenerateUUID( );
						asset->mLoader = query->second;
						info.mAsset = asset;
						info.mAssetName = asset->mName;
						info.mAssetUUID = asset->mUUID;
						info.mAssetFilePath = asset->mFilePath;
						info.mAssetLoadStatus = AssetLoadStatus::Loaded;

						// Add to loader
						query->second->AddToAssets( info );
					} 
				}
			}
		}

		// Result to return
		Result res = Result::SUCCESS;

		// If we need to cache the asset, then do that shit now
		if ( needToCache && cache )
		{
			// Handle serialization of asset file
			res = SerializeAsset( asset ); 
		}

		return res;
	}

	//======================================================================================================

	Result AssetManager::SerializeAsset( const Asset* asset )
	{
		// Serialize asset with archiver
		AssetArchiver archiver; 

		// Should the UUID of the asset be written here? Should it be in the ObjectArchiver serialize path?
		// Should there be a separate archiver that is in charge specifically of assets?  

		Result res = archiver.Serialize( asset ); 

		// Write to file using archiver 
		String path = mCachedPath + asset->mName + ".easset"; 
		archiver.WriteToFile( path );

		// Construct and add record to manifest
		CacheManifestRecord record;
		record.mAssetUUID = asset->mUUID;
		record.mAssetFilePath = path;
		record.mAssetLoaderClass = asset->mLoader->Class( );
		record.mAssetName = asset->mName;
		mCacheManifest.AddRecord( record ); 

		return res;
	}

	/*
		What is the process for deserializing a cached asset?
		On start of a project: 

		Project::OnLoad()
		{
			// Need to initialize asset manager in here, which sets the relative project directory, the contents directory, etc.
			InitiliazeAssetManager(); 

			// Inside of a project file, it has a UUID of a scene asset it needs to load as its "Start Scene"
			// If this scene cannot be found, as all other assets, will be load default scene asset

			// Get asset manager
			const AssetManager* am = Engine::GetInstance()->GetSubsystemCatalog()->Get< AssetManager >();
			
			// Load current scene from database
			mProfile.mCurrentScene = am->Get< Scene >( mProfile.mDefaultSceneId );	// Something like this, although less ugly...  
		}

		Project::InitilizeAssetManager()
		{
			// Need to set up project directory in here
			const AssetManager* am = Engine::GetInstance()->GetSubsystemCatalog()->Get< AssetManager >(); 

			// Initialize AssetManager with project directory
			// Only the project/engine should be able to initialize the asset manager!
			am->Initialize(mProfile.mProjectDirectory + "/Assets/"); 
		} 

		AssetManager::Initialize( const String& assetsDirectory ) const
		{
			// NOTE(): Clear all previous assets here - burn it all down

			// Set assets directory
			mAssetsDirectory = assetsDirectory; 

			// Load in cached asset manifest ( top level file )
			mCacheManifest =  CacheManifest( mAssetDirectory + "Intermediate/CacheRegistry.manifest" );

			// Collect all assets files from directory
			for ( auto& p : std::experimental::filesystem::recursive_directory_iterator( mAssetsDirectory ) )
			{
				// Grab asset path from p
				String assetPath = p.path().string();

				if ( HasFileExtension( p.path().string(), "easset" ) )
				{
					// Need to be able to determine which loader this asset uses to place appropriately
					// Need to create the wrapper for this asset to be held by the loader

					AssetRecordInfo record;
					record.mAssetLoadStatus = AssetLoadStatus::Unloaded;
					record.mAssetFilePath	= assetPath; 

					// Need to store unloaded assets by UUID, since that's how they will be looked up when loaded into memory by some other referencing asset...
					// However, the cached files are not cached with their UUID readily accessible - it's stored deep within the binary, which defeat the purpose of 
					//	quickly pre-loaded these assets
					// Perhaps this is where a cache registry comes into play? This is getting a bit out of hand, of course...

					// Maybe all unloaded assets are held by the asset manager directly? It's not until they're loaded in that they're passed
					//	directly to their respective loaders?
				} 
			} 
		}

		CacheManifest( const String& manifestPath )
		{
			// Store manifest path
			mManifestFilePath = manifestPath;

			// Read into buffer
			ByteBuffer readBuffer;
			readBuffer.ReadFromFile( mManifestFilePath );

			// Grab record count
			u32 recordCount = readBuffer.Read< u32 >(); 

			// For each record, store into manifest 
			for ( u32 i = 0; i < recordCount; ++i )
			{
				// Asset record to fill out
				AssetRecordInfo ar; 
				
				// Load into record
				ar.mUUID = readBuffer.Read< UUID >();
				ar.mAssetFilePath = readBuffer.Read< String >();

				// Get MetaClass for loader
				const MetaClass* cls = Object::GetClass( readBuffer.Read< String >() ); 
				ar.mLoaderClass = cls;

				// Add record
				mAssetRecords[ar.mUUID] = ar;
			}
		}

		CacheRegistryFile:
			u32 - Number of records stored
			for each record:
				UUID - UUID of asset
				String - File path of asset binary file (.easset) 
				MetaClass - MetaClass of loader this asset belongs to ( possibly )

		// This loads the scene, which is deserialized through the asset manager
		// This will look inside the loader to which this particular asset belongs... So how does THIS work exactly?


		Scene is loaded - This 


		Result AssetManager::DeserializeAsset( const String& filePath )
		{
			
		}
	*/

	//======================================================================================================

	bool AssetManager::HasFileExtension( const String& file, const String& extension )
	{
		return ( Enjon::Utils::SplitString( file, "." ).back( ).compare( extension ) == 0 );
	}

	//======================================================================================================

	Enjon::Result AssetManager::RegisterAssetLoaderInternal( AssetLoader* loader, u32 idx )
	{
		// Set into map
		mLoadersByAssetId[ idx ] = loader;

		// Get meta class of loader 
		const MetaClass* cls = mLoadersByAssetId[ idx ]->Class( );
		// Register by meta class
		mLoadersByMetaClass[ cls ] = mLoadersByAssetId[ idx ];

		// TODO(): This crashes for now. I want to set this here, so figure it out.
		// Register default asset from loader
		//mLoadersByAssetId[ idx ]->RegisterDefaultAsset( ); 

		return Result::SUCCESS;
	}

	//======================================================================================================

	bool AssetManager::Exists( u32 id ) const
	{
		return ( mLoadersByAssetId.find( id ) != mLoadersByAssetId.end( ) );
	}

	//======================================================================================================

	bool AssetManager::Exists( const MetaClass* cls ) const
	{
		return ( mLoadersByMetaClass.find( cls ) != mLoadersByMetaClass.end( ) );
	} 

	//======================================================================================================

	const AssetLoader* AssetManager::GetLoader( const MetaClass* cls ) const
	{
		if ( Exists( cls ) )
		{
			return const_cast< AssetManager* > ( this )->mLoadersByMetaClass[ cls ];
		}

		return nullptr;
	}

	//======================================================================================================

	const AssetLoader* AssetManager::GetLoaderByAssetClass( const MetaClass* cls )
	{
		// Get loader id from cls
		u32 loaderId = cls->GetTypeId( );

		if ( Exists( loaderId ) )
		{
			return mLoadersByAssetId[ loaderId ];
		}

		return nullptr;
	}

	//======================================================================================================
}







