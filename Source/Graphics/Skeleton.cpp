// @file Skeleton.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/Skeleton.h"
#include "Asset/MeshAssetLoader.h"

namespace Enjon
{ 
	//====================================================================

	Bone::Bone( )
	{ 
	}

	//====================================================================

	Bone::~Bone( )
	{ 
	}

	//====================================================================

	Skeleton::Skeleton( )
	{ 
	}

	//====================================================================

	Skeleton::~Skeleton( )
	{ 
	}
 
	//==================================================================== 

	bool Skeleton::HasBone( const String& name )
	{
		return ( mBoneNameLookup.find( name ) != mBoneNameLookup.end( ) );
	}

	//==================================================================== 
}