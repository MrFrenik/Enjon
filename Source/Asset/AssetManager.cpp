// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.cpp

#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "Asset/TextureAssetLoader.h" 
#include "Asset/MeshAssetLoader.h" 
#include "Asset/FontAssetLoader.h"
#include "Asset/ShaderGraphAssetLoader.h"
#include "Utils/FileUtils.h"
#include "Serialize/ObjectArchiver.h"
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
		RegisterAssetLoader< Enjon::Texture, TextureAssetLoader >( );
		RegisterAssetLoader< Enjon::Mesh, MeshAssetLoader >( );
		RegisterAssetLoader< Enjon::UIFont, FontAssetLoader >( );
		RegisterAssetLoader< Enjon::ShaderGraph, ShaderGraphAssetLoader >( );

		// Create file extension map
		mFileExtensionMap[ "png" ]	= GetAssetTypeId< Enjon::Texture >( );
		mFileExtensionMap[ "tga" ]	= GetAssetTypeId< Enjon::Texture >( );
		mFileExtensionMap[ "jpeg" ] = GetAssetTypeId< Enjon::Texture >( );
		mFileExtensionMap[ "bmp" ]	= GetAssetTypeId< Enjon::Texture >( );
		mFileExtensionMap[ "hdr" ]	= GetAssetTypeId< Enjon::Texture >( );
		mFileExtensionMap[ "fbx" ]	= GetAssetTypeId< Enjon::Mesh >( );
		mFileExtensionMap[ "obj" ]	= GetAssetTypeId< Enjon::Mesh >( );
		mFileExtensionMap[ "ttf" ]	= GetAssetTypeId< Enjon::UIFont >( );
		mFileExtensionMap[ "otf" ]	= GetAssetTypeId< Enjon::UIFont >( ); 
		mFileExtensionMap[ "sg" ]	= GetAssetTypeId< Enjon::ShaderGraph >( ); 
	}
	
	//============================================================================================ 
			
	Result AssetManager::Initialize()
	{ 
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
			
	Enjon::String AssetManager::GetAssetsPath( )
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
	
	//============================================================================================ 
			
	Result AssetManager::AddToDatabase( const String& filePath, b8 isRelativePath )
	{ 
		// Have to do a switch based on extension of file
		s32 idx = GetLoaderIdxByFileExtension( filePath );

		// Asset to be returned from loading if successful
		Asset* asset = nullptr;

		// If out of bounds, return failure since file extension was unknown
		if ( idx < 0 )
		{
			// TODO(): Log that extension was unknown
			return Result::FAILURE;
		}

		// Get qualified name of asset
		String qualifiedName = AssetLoader::GetQualifiedName( filePath );

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
					if ( !Utils::FileExists( mAssetsPath + filePath ) )
					{
						return Result::FAILURE;
					}

					String path = mAssetsPath + filePath;
					String name = Utils::ToLower( mName ) + qualifiedName; 

					asset = query->second->LoadResourceFromFile( path, name ); 

					// Set file path and name
					if ( asset )
					{
						asset->mFilePath = mAssetsPath + filePath;
						asset->mName = name; 
						asset->mUUID = UUID::GenerateUUID( );
					}
				}

				// If absolute path on disk
				else
				{
					// Return failure if path doesn't exist
					if ( !Utils::FileExists( filePath ) )
					{
						return Result::FAILURE;
					}

					asset = query->second->LoadResourceFromFile( filePath, qualifiedName );
				}
			}
		}

		// Handle serialization of asset file
		SerializeAsset( asset );

		return Result::SUCCESS;
	}

	//======================================================================================================

	Result AssetManager::SerializeAsset( const Asset* asset )
	{
		// Serialize asset with archiver
		ObjectArchiver archiver;
		Result res = archiver.Serialize( asset ); 

		// Write to file using archiver 
		String path = mCachedPath + asset->mName + ".easset"; 
		archiver.WriteToFile( path );

		return res;
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

	const AssetLoader* AssetManager::GetLoader( const MetaClass* cls )
	{
		if ( Exists( cls ) )
		{
			return mLoadersByMetaClass[ cls ];
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







