// @file Skeleton.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/Skeleton.h"
#include "Asset/MeshAssetLoader.h"
#include "Asset/SkeletalMeshAssetLoader.h"
#include "Asset/SkeletonAssetLoader.h"
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

	bool Skeleton::HasJoint( const String& name ) const
	{
		return ( mJointNameLookup.find( name ) != mJointNameLookup.end( ) );
	} 

	//==================================================================== 

	Vector< Mat4x4 > Skeleton::GetTransforms( const AssetHandle< SkeletalAnimation >& animationHandle, const f32& time ) const
	{
		Vector< Mat4x4 > matrices;

		if ( mJoints.empty( ) || mRootID == -1 )
		{
			return matrices;
		}

		matrices.resize( mJoints.size( ) );
		
		CalculateTransform( mRootID, Mat4x4::Identity( ), matrices, animationHandle.Get(), time );

		return matrices;
	}

	//==================================================================== 

	void Skeleton::CalculateTransform( const u32& jointID, const Mat4x4& parentMatrix, Vector<Mat4x4>& outMatrices, const SkeletalAnimation* animation, const f32& time ) const
	{
		// Get joint
		const Joint* joint = &mJoints.at( jointID );

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
			outMatrices.at( joint->mID ) = mGlobalInverseTransform * joint->mInverseBindMatrix;
		}

		// Iterate through children 
		for ( u32 i = 0; i < joint->mChildren.size(); ++i )
		{
			CalculateTransform( joint->mChildren.at( i ), relativeTransform, outMatrices, animation, time );				
		}
	} 

	//==================================================================== 

	u32 Skeleton::GetNumberOfJoints( ) const
	{
		return mJoints.size( );
	}

	//==================================================================== 

	s32 Skeleton::GetJointIndex( const String& name ) const
	{
		if ( HasJoint( name ) )
		{
			return mJointNameLookup.at( name );
		}

		return -1;
	}

	//==================================================================== 

	Result Skeleton::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out root id
		buffer->Write< s32 >( mRootID );

		// Write out size of joints
		buffer->Write< usize >( GetNumberOfJoints( ) );

		// Write out joints
		for ( auto& j : mJoints )
		{
			j.SerializeData( buffer );
		}

		// Write out joint name lookup map
		for ( auto& j : mJointNameLookup )
		{
			buffer->Write< String >( j.first );
			buffer->Write< u32 >( j.second );
		}

		// Write out global inverse matrix
		buffer->Write< Mat4x4 >( mGlobalInverseTransform );

		return Result::SUCCESS;
	}

	//==================================================================== 

	Result Skeleton::DeserializeData( ByteBuffer* buffer )
	{
		// Read in root id
		mRootID = buffer->Read< s32 >( ); 

		// Resize number of joints
		mJoints.resize( buffer->Read< usize >( ) );

		// Write out joints
		for ( usize i = 0; i < mJoints.size( ); ++i )
		{
			mJoints.at( i ).DeserializeData( buffer );
		}

		// Read in joint map name data
		for ( u32 i = 0; i < mJoints.size( ); ++i )
		{
			String key = buffer->Read< String >( );
			u32 val = buffer->Read< u32 >( );
			mJointNameLookup[key] = val;
		}

		// Read in global inverse matrix
		mGlobalInverseTransform = buffer->Read< Mat4x4 >( );

		return Result::SUCCESS;
	}

	//==================================================================== 

	Result Joint::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out parent id
		buffer->Write< s32 >( mParentID );

		// Write out children id size
		buffer->Write< usize >( mChildren.size( ) );

		// Write out children ids
		for ( auto& c : mChildren )
		{
			buffer->Write< u32 >( c );
		}

		// Write out id
		buffer->Write< u32 >( mID );

		// Write out inverse bind matrix
		buffer->Write< Mat4x4 >( mInverseBindMatrix ); 

		return Result::SUCCESS;
	}

	//==================================================================== 

	Result Joint::DeserializeData( ByteBuffer* buffer )
	{
		// Read in parent id
		mParentID = buffer->Read< s32 >( );

		// Read in children size
		mChildren.resize( buffer->Read< usize >( ) );

		// Read in children ids
		for ( usize i = 0; i < mChildren.size( ); ++i )
		{
			mChildren.at( i ) = buffer->Read< u32 >( );
		}

		// Read in id
		mID = buffer->Read< u32 >( );

		// Read in inverse bind matrix
		mInverseBindMatrix = buffer->Read< Mat4x4 >( );

		return Result::SUCCESS;
	}

	//==================================================================== 

}


