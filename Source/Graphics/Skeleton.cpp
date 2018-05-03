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

	Joint::Joint( )
	{ 
	}

	//====================================================================

	Joint::~Joint( )
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

	bool Skeleton::HasJoint( const String& name )
	{
		return ( mJointNameLookup.find( name ) != mJointNameLookup.end( ) );
	}

	//==================================================================== 

	Vector< Mat4x4 > Skeleton::GetTransforms( )
	{
		Vector< Mat4x4 > matrices;
		matrices.resize( mJoints.size( ) );
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

	void Skeleton::CalculateTransform( const u32& jointID, const Mat4x4& parentMatrix, Vector<Mat4x4>& outMatrices, SkeletalAnimation* animation, const f32& time )
	{
		// Get joint
		Joint* joint = &mJoints.at( jointID );

		// Calculate bone transform ( bone space ) // Identity for now ( No animation )
		Mat4x4 jointTransform = Mat4x4::Identity( );
		if ( animation )
		{
			Transform t =  animation->CalculateInterpolatedTransform( time, jointID );
			jointTransform = t.ToMat4x4( );
		} 

		// Calculate relative to parent
		Mat4x4 relativeTransform = parentMatrix * jointTransform;

		// Calculate and set local space matrix
		if ( animation != nullptr )
		{
			outMatrices.at( joint->mID ) = mGlobalInverseTransform * relativeTransform * joint->mInverseBindMatrix; 
		}
		else
		{
			outMatrices.at( joint->mID ) = jointTransform;
		}

		// Iterate through children 
		for ( u32 i = 0; i < joint->mChildren.size(); ++i )
		{
			CalculateTransform( joint->mChildren.at( i ), relativeTransform, outMatrices, animation, time );				
		}
	} 
}


