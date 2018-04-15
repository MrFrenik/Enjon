// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.cpp

#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "Asset/TextureAssetLoader.h" 
#include "Asset/MeshAssetLoader.h" 
#include "Asset/FontAssetLoader.h"
#include "Asset/MaterialAssetLoader.h"
#include "Asset/ShaderGraphAssetLoader.h"
#include "Asset/SceneAssetLoader.h"
#include "Utils/FileUtils.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/AssetArchiver.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

#include <fmt/printf.h>
#include <filesystem>

namespace FS = std::experimental::filesystem; 

namespace Enjon
{
	//============================================================================================ 

	AssetManager::AssetManager( )
	{
		// Set to default
		mName = "";

		// Set to default
		mAssetsDirectoryPath = "";

		// Register the loaders with manager 
		RegisterLoaders( );
	}

	//============================================================================================ 

	AssetManager::AssetManager( const String& name, const String& assetsPath )
	{
		// Set name
		mName = name;

		// Set project asset path
		mAssetsDirectoryPath = assetsPath;

		// Register the loaders with manager 
		RegisterLoaders( );
	}

	//============================================================================================ 

	void AssetManager::RegisterLoaders( )
	{
		// Register the loaders with manager 
		RegisterAssetLoader< Texture, TextureAssetLoader >( );
		RegisterAssetLoader< Mesh, MeshAssetLoader >( );
		RegisterAssetLoader< UIFont, FontAssetLoader >( );
		RegisterAssetLoader< ShaderGraph, ShaderGraphAssetLoader >( );
		RegisterAssetLoader< Material, MaterialAssetLoader >( );
		RegisterAssetLoader< Scene, SceneAssetLoader >( );

		// Create file extension map
		mFileExtensionMap[ "png" ] = GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "tga" ] = GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "jpeg" ] = GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "bmp" ] = GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "hdr" ] = GetAssetTypeId< Texture >( );
		mFileExtensionMap[ "fbx" ] = GetAssetTypeId< Mesh >( );
		mFileExtensionMap[ "obj" ] = GetAssetTypeId< Mesh >( );
		mFileExtensionMap[ "ttf" ] = GetAssetTypeId< UIFont >( );
		mFileExtensionMap[ "otf" ] = GetAssetTypeId< UIFont >( );
		mFileExtensionMap[ "sg" ] = GetAssetTypeId< ShaderGraph >( );
	}

	//============================================================================================ 

	// Base initialization method called from engine
	Result AssetManager::Initialize( )
	{
		// Initialize the manifest and read in records
		mCacheManifest.Initialize( mAssetsDirectoryPath + "/Intermediate/CacheManifest.bin", this );

		// NOTE(): After first initialization, set asset location type to be application specific
		// NOTE(): I hate this, by the way...
		mAssetLocationType = AssetLocationType::ApplicationAsset;


		return Result::SUCCESS;
	}

	//============================================================================================ 

	void AssetManager::Reinitialize( const String& assetsPath )
	{
		// Clear records for loaders
		for ( auto& l : mLoadersByAssetId )
		{
			l.second->ClearRecords( );
		}

		// Set assets directory path as well as cache path
		SetAssetsDirectoryPath( assetsPath );

		// Call initialize function
		Initialize( );
	}

	//============================================================================================ 

	AssetLocationType AssetManager::GetAssetLocationType( ) const
	{
		return mAssetLocationType;
	}

	//============================================================================================ 

	void AssetManager::Update( const f32 dT )
	{
		// In here could check for file updates on hot loaded resources
	}

	//============================================================================================ 

	Result AssetManager::Shutdown( )
	{
		// Delete all asset loaders
		for ( auto& l : mLoadersByAssetId )
		{
			delete l.second;
			l.second = nullptr;
		}

		mLoadersByAssetId.clear( );
		mLoadersByMetaClass.clear( );

		// Reset cache registry manifest
		mCacheManifest.Reset( );

		return Result::SUCCESS;
	}

	//============================================================================================ 

	void AssetManager::SetAssetsDirectoryPath( const String& path )
	{
		mAssetsDirectoryPath = path;
		SetCachedAssetsDirectoryPath( mAssetsDirectoryPath + "Cache/" );
	}

	//============================================================================================ 

	Enjon::String AssetManager::GetAssetsDirectoryPath( ) const
	{
		return mAssetsDirectoryPath;
	}

	//============================================================================================ 

	void AssetManager::SetCachedAssetsDirectoryPath( const String& filePath )
	{
		mCachedDirectoryPath = filePath;
	}

	//============================================================================================ 

	const Enjon::String& AssetManager::GetCachedAssetsDirectoryPath( ) const
	{
		return mCachedDirectoryPath;
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
		String fileExtension = Utils::ToLower( Utils::SplitString( filePath, "." ).back( ) );

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

	Asset* AssetManager::GetDefaultAsset( const Enjon::MetaClass* cls ) const
	{
		// Make sure class is valid
		assert( cls != nullptr );

		// Get type id of class
		u32 idx = cls->GetTypeId( );

		if ( Exists( idx ) )
		{
			return ConstCast< AssetManager >( )->mLoadersByAssetId[ idx ]->GetDefault( );
		}

		return nullptr;
	}

	//============================================================================================ 

	const Asset* AssetManager::GetAsset( const MetaClass* cls, const UUID& id ) const
	{
		// Get type id of class
		u32 idx = cls->GetTypeId( );

		if ( Exists( idx ) )
		{
			return const_cast< AssetManager* >( this )->mLoadersByAssetId[ idx ]->GetAsset( id );
		}

		return nullptr;
	}

	//============================================================================================ 

	const HashMap< Enjon::String, AssetRecordInfo >* AssetManager::GetAssets( const Enjon::MetaClass* cls ) const
	{
		// Make sure class is valid
		assert( cls != nullptr );

		// Get type id of class
		u32 idx = cls->GetTypeId( );

		if ( Exists( idx ) )
		{
			return ConstCast< AssetManager >( )->mLoadersByAssetId[ idx ]->GetAssets( );
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

	Result AssetManager::AddToDatabase( const String& resourceFilePath, const String& destDir, bool cache, AssetLocationType locationType )
	{
		Result res = Result::SUCCESS;

		bool needToCache = false;

		// Get idx of loader
		s32 idx = GetLoaderIdxByFileExtension( resourceFilePath );

		Asset* asset = nullptr;

		// Load not found
		if ( idx < 0 )
		{
			return Result::FAILURE;
		}

		// Get original name of asset
		String resourceFilePathName = Utils::FindReplaceAll( resourceFilePath, "\\", "/" );
		Vector<String> splits = Utils::SplitString( resourceFilePathName, "/" );
		// Get back of splits for file name
		resourceFilePathName = splits.back( );

		// Get asset name
		String assetDisplayName = Utils::SplitString( resourceFilePathName, "." ).at( 0 );

		// Construct asset path
		String destAssetPath = Utils::FindReplaceAll( destDir + "/" + resourceFilePathName, "\\", "/" );
 
		// Get qualified name of asset
		String qualifiedName = AssetLoader::GetQualifiedName( destAssetPath );

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

				// Return failure if path doesn't exist
				if ( !Utils::FileExists( resourceFilePath ) )
				{
					return Result::FAILURE;
				}

				//// If file exists, grab that
				//if ( query->second->Exists( Utils::ToLower( mName ) + qualifiedName ) )
				//{
				//	return Result::SUCCESS;
				//}

				else
				{
					// We need to cache the asset at the end of this operation
					needToCache = true;

					// Load the asset from file
					asset = query->second->LoadResourceFromFile( resourceFilePath );

					// If asset is valid
					if ( asset )
					{
						// Add to loader assets with asset record info
						AssetRecordInfo info;
						asset->mName = qualifiedName;
						asset->mFilePath = destAssetPath;				// THIS IS INCORRECT! NEED TO CHANGE TO BEING THE ACTUAL CACHED ASSET PATH!
						asset->mUUID = UUID::GenerateUUID( );
						asset->mLoader = query->second;
						info.mAsset = asset;
						info.mAssetName = asset->mName;
						info.mAssetUUID = asset->mUUID;
						info.mAssetFilePath = destAssetPath;							// THIS IS INCORRECT! NEED TO CHANGE TO BEING THE ACTUAL CACHED ASSET PATH!
						info.mAssetDisplayName = assetDisplayName;
						info.mAssetLoadStatus = AssetLoadStatus::Loaded;

						// Add to loader
						query->second->AddToAssets( info );
					}
				}
			}
		}

		// If we need to cache the asset, then do that shit now
		if ( needToCache && cache )
		{
			// Handle serialization of asset file
			res = SerializeAsset( asset, qualifiedName, destDir );
		}

		return res;
	}
 
	//============================================================================================ 
			
	Result AssetManager::AddToDatabase( const String& resourceFilePath, bool cache, bool isRelativePath, AssetLocationType locationType )
	{ 
		// Result to be returned
		Result res = Result::SUCCESS;

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
					if ( !Utils::FileExists( mAssetsDirectoryPath + resourceFilePath ) )
					{
						return Result::FAILURE;
					} 

					// If file exists, grab that
					if ( query->second->Exists( Utils::ToLower( mName ) + qualifiedName ) )
					{
						return Result::SUCCESS;
					}

					else
					{
						// We need to cache the asset at the end of this operation
						needToCache = true;

						// Load the asset from file
						asset = query->second->LoadResourceFromFile( mAssetsDirectoryPath + resourceFilePath );
	 
						// If asset is valid
						if ( asset )
						{
							// Add to loader assets with asset record info
							AssetRecordInfo info;
							asset->mName =  qualifiedName;
							asset->mFilePath = mAssetsDirectoryPath + resourceFilePath;				// THIS IS INCORRECT! NEED TO CHANGE TO BEING THE ACTUAL CACHED ASSET PATH!
							asset->mUUID = UUID::GenerateUUID( );
							asset->mLoader = query->second;
							info.mAsset = asset;
							info.mAssetName = asset->mName;
							info.mAssetUUID = asset->mUUID;
							info.mAssetFilePath = asset->mFilePath;							// THIS IS INCORRECT! NEED TO CHANGE TO BEING THE ACTUAL CACHED ASSET PATH!
							info.mAssetLoadStatus = AssetLoadStatus::Loaded;
							info.mAssetDisplayName = asset->mName;

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

					// If file exists, grab that
					if ( query->second->Exists( qualifiedName ) )
					{
						return Result::SUCCESS;
					}

					else
					{
						// Need to cache file 
						needToCache = true;

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
							info.mAssetDisplayName = asset->mName;

							// Add to loader
							query->second->AddToAssets( info );
						} 
					} 
				}
			}
		} 

		// If we need to cache the asset, then do that shit now
		if ( needToCache && cache )
		{
			// Handle serialization of asset file
			res = SerializeAsset( asset, qualifiedName ); 
		}

		return res;
	}

	//======================================================================================================

	Result AssetManager::SerializeAsset( const Asset* asset, const String& assetName, const String& path )
	{
		// Serialize asset with archiver
		AssetArchiver archiver; 

		// Should the UUID of the asset be written here? Should it be in the ObjectArchiver serialize path?
		// Should there be a separate archiver that is in charge specifically of assets?  

		Result res = archiver.Serialize( asset ); 

		// Get file extension from loader
		String fileExtension = asset->mLoader->GetAssetFileExtension( );

		// Write to file using archiver 
		String assetPath = path;

		// If path is empty, then set assetPath to cached directory
		if ( path.compare( "" ) == 0 )
		{
			assetPath = mCachedDirectoryPath + asset->GetAssetRecordInfo()->GetAssetDisplayName() + fileExtension; 
		}
		else
		{
			assetPath = path + "/" + asset->GetAssetRecordInfo()->GetAssetDisplayName() + fileExtension;
		}

		// Write the binary to file
		archiver.WriteToFile( Utils::FindReplaceAll( assetPath, "\\", "/" ) );

		// Construct and add record to manifest
		CacheManifestRecord record;
		record.mAssetUUID = asset->mUUID;
		record.mAssetFilePath = assetPath;
		record.mAssetLoaderClass = asset->mLoader->Class( );
		record.mAssetName = asset->mName;
		mCacheManifest.AddRecord( record ); 

		return res;
	}

	//======================================================================================================

	Result AssetManager::SaveAsset( const Asset* asset ) const
	{
		// Can only save asset if it's valid and NOT a default engine asset
		if ( asset && !asset->IsDefault() )
		{
			const AssetRecordInfo* info = asset->GetAssetRecordInfo( );
			if ( info )
			{
				AssetArchiver archiver;
				Result res = archiver.Serialize( asset );
				archiver.WriteToFile( info->GetAssetFilePath( ) );

				return Result::SUCCESS;
			}
			else
			{
				return Result::FAILURE;
			}
		}
		else
		{
			return Result::FAILURE;
		}
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

	bool AssetManager::HasAnyAssetFileExtension( const String& file )
	{
		AssetManager* am = EngineSubsystem( AssetManager ); 
		if ( am )
		{
			// Get file extension of passed in file
			String fileExt = "." + Enjon::Utils::SplitString( file, "." ).back( );

			// Check for matches
			for ( auto& l : am->mLoadersByAssetId )
			{ 
				if ( fileExt.compare( l.second->GetAssetFileExtension( ) ) == 0 )
				{
					return true;
				}
			}
		}

		// Couldn't file match
		return false; 
	}

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

	const AssetLoader* AssetManager::GetLoaderByAssetClass( const MetaClass* cls ) const
	{
		// Get loader id from cls
		u32 loaderId = cls->GetTypeId( );

		if ( Exists( loaderId ) )
		{
			return ConstCast< AssetManager >()->mLoadersByAssetId[ loaderId ];
		}

		return nullptr;
	}

	//======================================================================================================

	AssetHandle< Asset > AssetManager::GetAssetFromFilePath( const String& path )
	{
		// For each loader
		for ( auto& l : mLoadersByAssetId )
		{
			// For each asset record in loader
			for ( auto& r : l.second->mAssetsByUUID )
			{
				// If the path's are identical, then we've hit gold
				if ( FS::path( Utils::FindReplaceAll( Utils::FindReplaceAll( r.second.GetAssetFilePath( ), "\\", "/" ), "//", "/" ) ) == FS::path( path ) )
				{
					return r.second.GetAsset( );
				}
			}
		}

		// If not found, return blank asset handle
		return AssetHandle< Asset >( );
	}

	//======================================================================================================

	void AssetManager::RenameAssetFilePath( const AssetHandle< Asset >& asset, const String& path )
	{
		if ( asset.Get() )
		{
			// Grab the asset from the handle
			Asset* a = asset.Get( )->ConstCast< Asset >();

			// Update in the loader
			a->GetLoader( )->ConstCast< AssetLoader >( )->RenameAssetFilePath( a, path ); 
		}
	}

	//======================================================================================================
}


/*
	// The only thing scenes will do is hold onto binary/json data to be loaded... they don't hold onto run time information...
	ENJON_CLASS( Construct )
	class Scene : public Asset
	{ 
		ENJON_CLASS_BODY( )

		public:

		protected:

		private:
	}; 

	// Need to be able to save/load scenes while in a project from its asset database
	// Cannot save scene at runtime
	// Can load scenes either at runtime or at edit-time
	// Need to be able to create new entities during edit-time
*/




































