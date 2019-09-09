// @file Skeleton.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/Skeleton.h"
#include "Asset/MeshAssetLoader.h"
#include "Asset/SkeletalMeshAssetLoader.h"
#include "Asset/SkeletonAssetLoader.h"
#include "Graphics/SkeletalAnimation.h"
#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/MeshAssetLoader.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{ 
 
	//==================================================================== 

	bool Skeleton::HasJoint( const String& name ) const
	{
		return ( mJointNameLookup.find( name ) != mJointNameLookup.end( ) );
	} 

	//==================================================================== 

	void Skeleton::GetBindJointTransforms( Vector< Mat4x4 >& outMatrices ) const
	{ 
		if ( mJoints.empty( ) || mRootID == -1 )
		{
			return;
		} 

		outMatrices.resize( mJoints.size( ) );

		for ( u32 i = 0; i < mJoints.size( ); ++i )
		{ 
			//outMatrices.at( mJoints[ i ].mID ) = mGlobalInverseTransform * mJoints[ i ].mInverseBindMatrix; 
			outMatrices.at( mJoints[ i ].mID ) = mGlobalInverseTransform;
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
		buffer->Write< u32 >( (u32)GetNumberOfJoints( ) );

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
		mJoints.resize( buffer->Read< u32 >( ) );

		// Write out joints
		for ( u32 i = 0; i < (u32)mJoints.size( ); ++i )
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

	s32 Skeleton::GetRootID( ) const
	{
		return mRootID;
	}

	//==================================================================== 

	Result Joint::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out parent id
		buffer->Write< s32 >( mParentID );

		// Write out children id size
		buffer->Write< u32 >( (u32)mChildren.size( ) );

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
		mChildren.resize( buffer->Read< u32 >( ) );

		// Read in children ids
		for ( u32 i = 0; i < (u32)mChildren.size( ); ++i )
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


