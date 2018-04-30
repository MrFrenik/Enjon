// @file SkeletonAssetLoader.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Asset/SkeletonAssetLoader.h"

namespace Enjon
{
	//=========================================================================

	SkeletonAssetLoader::SkeletonAssetLoader( )
	{ 
	}

	//=========================================================================

	SkeletonAssetLoader::~SkeletonAssetLoader( )
	{ 
	}

	//=========================================================================

	void SkeletonAssetLoader::RegisterDefaultAsset( )
	{ 
	}

	//=========================================================================

	Asset* SkeletonAssetLoader::LoadResourceFromFile( const String& filePath )
	{ 
		Skeleton* skeleton = new Skeleton( );

		return skeleton;
	}

	//=========================================================================
}
