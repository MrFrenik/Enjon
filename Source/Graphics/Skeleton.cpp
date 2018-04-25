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

	Vector< Mat4x4 > Skeleton::GetTransforms( )
	{
		Vector< Mat4x4 > matrices;
		matrices.resize( mBones.size( ) );
		CalculateTransform( mRootID, Mat4x4::Identity( ), matrices );
		return matrices;
	}

	//==================================================================== 

	void Skeleton::CalculateTransform( const u32& boneID, const Mat4x4& parentMatrix, Vector<Mat4x4>& outMatrices )
	{
		// Get bone
		Bone* bone = &mBones.at( boneID );

		// Calculate bone transform ( bone space ) // Identity for now ( No animation )
		Mat4x4 boneTransform = Mat4x4::Identity( );
		//Mat4x4 boneTransform = matTrans * matRotation * matScale;

		// Calculate relative to parent
		Mat4x4 relativeTransform = parentMatrix * boneTransform;

		// Calculate and set local space matrix
		outMatrices.at( bone->mID ) = mGlobalInverseTransform * relativeTransform * bone->mInverseBindMatrix;

		// Iterate through children
		for ( u32 i = 0; i < bone->mChildren.size(); ++i )
		{
			CalculateTransform( bone->mChildren.at( i ), relativeTransform, outMatrices );				
		}
	} 
}


