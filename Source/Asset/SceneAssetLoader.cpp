// File: SceneAssetLoader.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Asset/SceneAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//=======================================================================================

	void SceneAssetLoader::RegisterDefaultAsset( )
	{ 
		// Create new graph and compile
		Scene* scene = new Scene( );
		scene->mName = "DefaultScene"; 

		// Set default asset
		mDefaultAsset = scene;
	}

	//=======================================================================================

	Asset* SceneAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset;
	}

	//=======================================================================================

	String SceneAssetLoader::GetAssetFileExtension( ) const
	{
		return ".escene";
	}
}
