// File: ArchetypeAssetLoader.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Asset/ArchetypeAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//=======================================================================================

	void ArchetypeAssetLoader::RegisterDefaultAsset( )
	{ 
		Archetype* archetype = new Archetype( ); 

		// Construct new empty entity in archetype?
		archetype->mName = "DefaultArchetype"; 

		// Set default asset
		mDefaultAsset = archetype;
	}

	//=======================================================================================

	Asset* ArchetypeAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset;
	}

	//=======================================================================================

	String ArchetypeAssetLoader::GetAssetFileExtension( ) const
	{
		return ".earchetype";
	}
}
