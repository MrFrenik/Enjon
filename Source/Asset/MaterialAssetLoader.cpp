// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MaterialAssetLoader.cpp

#include "Asset/MaterialAssetLoader.h"
#include "Graphics/ShaderGraph.h"
#include "Graphics/Material.h"
#include "Asset/AssetManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//=======================================================================================

	void MaterialAssetLoader::RegisterDefaultAsset( )
	{ 
		// Create new graph and compile
		Material* material = new Material( );
		material->mShaderGraph = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->GetDefaultAsset< ShaderGraph >( );
		material->mName = "DefaultMaterial";

		// Set default asset
		mDefaultAsset = material;
	}

	//=======================================================================================

	Asset* MaterialAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset;
	}

	//=======================================================================================

	String MaterialAssetLoader::GetAssetFileExtension( ) const
	{
		return ".emat";
	}

	//=======================================================================================
}

