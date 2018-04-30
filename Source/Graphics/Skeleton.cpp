// @file Skeleton.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/Skeleton.h"
#include "Asset/MeshAssetLoader.h"
#include "Graphics/SkeletalAnimation.h"
#include "Asset/AssetManager.h"
#include "Asset/MeshAssetLoader.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

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
		SkeletalAnimation* animation = EngineSubsystem( AssetManager )->GetLoader( Object::GetClass< MeshAssetLoader >( ) )->Cast< MeshAssetLoader >( )->GetAnimation( 0 );
		static f32 t = 0.0f; 
		f32 dt = Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( );
		if ( animation )
		{ 
			t = std::fmod( t + dt, animation->GetDuration( ) );
		}
		CalculateTransform( mRootID, Mat4x4::Identity( ), matrices, animation, t );
		return matrices;
	}

	//==================================================================== 

	void Skeleton::CalculateTransform( const u32& boneID, const Mat4x4& parentMatrix, Vector<Mat4x4>& outMatrices, SkeletalAnimation* animation, const f32& time )
	{
		// Get bone
		Bone* bone = &mBones.at( boneID );

		// Calculate bone transform ( bone space ) // Identity for now ( No animation )
		Mat4x4 boneTransform = Mat4x4::Identity( );
		if ( animation )
		{
			Transform t =  animation->CalculateInterpolatedTransform( time, boneID );
			boneTransform = t.ToMat4x4( );
		}

		// Calculate relative to parent
		Mat4x4 relativeTransform = parentMatrix * boneTransform;

		// Calculate and set local space matrix
		outMatrices.at( bone->mID ) = mGlobalInverseTransform * relativeTransform * bone->mInverseBindMatrix;

		// Iterate through children
		for ( u32 i = 0; i < bone->mChildren.size(); ++i )
		{
			CalculateTransform( bone->mChildren.at( i ), relativeTransform, outMatrices, animation, time );				
		}
	} 
}


