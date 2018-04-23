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


/*
	// Used to calculate local space transform for all bones
	void CalculateTransform( const u32& boneID, const Mat4x4& parentMatrix, const Skeleton* skeleton, Vector<Matrix4x4>& outMatrices )
	{
		// Get bone
		Bone* bone = skeleton->mBones.at( bondID );

		// Calculate bone transform ( bone space )
		Mat4x4 boneTransform = matTrans * matRotation * matScale;

		// Calculate relative to parent
		Mat4x4 relativeTransform = parentMatrix * boneTransform;

		// Calculate and set local space matrix
		outMatrices.at( bone->mID ) = skeleton->mGlobalInverseTransform * relativeTransform * bone->mOffsetTransform;

		// Iterate through children
		for ( u32 i = 0; i < bone->mChildren; ++i )
		{
			CalculateTransform( bone->mChildren.at( i ), relativeTransform, skeleton, outMatrices );				
		}
	} 

*/