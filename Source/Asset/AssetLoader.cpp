// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.cpp

#include "Asset/AssetLoader.h"
#include "Utils/FileUtils.h"

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
			delete asset.second;
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

	/**
	* @brief 
	*/
	Asset* AssetLoader::GetAsset( UUID id )
	{
		if ( Exists( id ) )
		{
			return mAssetsByUUID[ id.ToString() ];
		}

		return nullptr;
	}
	
	//=================================================================
			
	bool AssetLoader::Exists( const String& name )
	{
		auto query = mAssetsByName.find( name );
		if ( query != mAssetsByName.end() )
		{
			return true;
		}

		return false;
	}
	
	//=================================================================
			
	bool AssetLoader::Exists( UUID uuid )
	{
		auto query = mAssetsByUUID.find( uuid.ToString() );
		if ( query != mAssetsByUUID.end( ) )
		{
			return true;
		}

		return false;
	} 
}

