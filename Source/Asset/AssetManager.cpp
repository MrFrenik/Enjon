// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.cpp

#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "Asset/TextureAssetLoader.h" 
#include "Asset/MeshAssetLoader.h" 
#include "Asset/FontAssetLoader.h"
#include "Asset/ShaderGraphAssetLoader.h"
#include "Utils/FileUtils.h"
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
		for (auto& loader : mLoaders)
		{
			delete loader.second;
		}

		// Clear map
		mLoaders.clear();
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
			
	const std::unordered_map< Enjon::String, Asset* >* AssetManager::GetAssets( const Enjon::MetaClass* cls )
	{
		// Make sure class is valid
		assert( cls != nullptr );

		// Get type id of class
		u32 idx = cls->GetTypeId( );

		if ( Exists( idx ) )
		{
			return mLoaders[ idx ]->GetAssets( );
		}

		return nullptr;
	}
	
	//============================================================================================ 
			
	Result AssetManager::AddToDatabase( const String& filePath, b8 isRelativePath )
	{ 
		// Have to do a switch based on extension of file
		s32 idx = GetLoaderIdxByFileExtension( filePath );

		// If out of bounds, return failure since file extension was unknown
		if ( idx < 0 )
		{
			// TODO(): Log that extension was unknown
			return Result::FAILURE;
		}

		// Get qualified name of asset
		String qualifiedName = AssetLoader::GetQualifiedName( filePath );

		// Find loader by idx
		auto query = mLoaders.find( ( u32 )idx );
		if ( query != mLoaders.end( ) )
		{
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

					auto res = query->second->LoadResourceFromFile( mAssetsPath + filePath, Utils::ToLower( mName ) + qualifiedName ); 
					
					// Set file path and name
					if ( res )
					{
						res->mFilePath = mAssetsPath + filePath;
						res->mName = Utils::ToLower( mName ) + qualifiedName;
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

					auto res = query->second->LoadResourceFromFile( filePath, qualifiedName );
				}
			}
		}

		return Result::SUCCESS;
	} 

	//======================================================================================================
			
	bool AssetManager::HasFileExtension( const String& file, const String& extension )
	{
		return ( Enjon::Utils::SplitString( file, "." ).back( ).compare( extension ) == 0 );
	}
}







