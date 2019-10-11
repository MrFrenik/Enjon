// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: UIAssetLoader.cpp

#include "Asset/UIAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//=======================================================================================

	void UIAssetLoader::RegisterDefaultAsset( )
	{ 
		// Set default asset
		mDefaultAsset = new UI();
	}

	//=======================================================================================

	Asset* UIAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset;
	}

	//=======================================================================================

	String UIAssetLoader::GetAssetFileExtension( ) const
	{
		return ".eui";
	}

	//=======================================================================================
}
