// @file SkeletalAnimation.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/SkeletalAnimation.h"
#include "Asset/SkeletalAnimationAssetLoader.h"
#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Graphics/Skeleton.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{ 
	//=============================================================== 

	Result ChannelData::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out rotation key size
		buffer->Write< usize >( mRotationKeys.size( ) );

		// Write out rotation keys
		for ( auto& k : mRotationKeys )
		{
			// Time stamp
			buffer->Write< f32 >( k.mTimeStamp );

			// Quaternion rotation
			buffer->Write< f32 >( k.mValue.x );
			buffer->Write< f32 >( k.mValue.y );
			buffer->Write< f32 >( k.mValue.z );
			buffer->Write< f32 >( k.mValue.w );
		}

		// Write out position key size
		buffer->Write< usize >( mPositionKeys.size( ) ); 

		// Write out position keys
		for ( auto& k : mPositionKeys )
		{
			// Time stamp
			buffer->Write< f32 >( k.mTimeStamp );

			// Vec3 position
			buffer->Write< f32 >( k.mValue.x );
			buffer->Write< f32 >( k.mValue.y );
			buffer->Write< f32 >( k.mValue.z );
		}

		// Write out scale key size
		buffer->Write< usize >( mScaleKeys.size( ) ); 

		// Write out position keys
		for ( auto& k : mScaleKeys )
		{
			// Time stamp
			buffer->Write< f32 >( k.mTimeStamp );

			// Vec3 scale
			buffer->Write< f32 >( k.mValue.x );
			buffer->Write< f32 >( k.mValue.y );
			buffer->Write< f32 >( k.mValue.z ); 
		} 

		return Result::SUCCESS;
	}

	//=============================================================== 
	
	Result ChannelData::DeserializeData( ByteBuffer* buffer )
	{
		// Read in rotation key size
		mRotationKeys.resize( buffer->Read< usize >( ) );

		// Write out rotation keys
		for ( usize i = 0; i < mRotationKeys.size( ); ++i )
		{
			// Grab reference to keyframe
			KeyFrame< Quaternion >& k = mRotationKeys.at( i ); 
			
			// Time stamp
			k.mTimeStamp = buffer->Read< f32 >( );

			// Quaternion rotation
			k.mValue.x = buffer->Read< f32 >( );
			k.mValue.y = buffer->Read< f32 >( );
			k.mValue.z = buffer->Read< f32 >( );
			k.mValue.w = buffer->Read< f32 >( );
		}

		// Read in rotation key size
		mPositionKeys.resize( buffer->Read< usize >( ) );

		// Write out rotation keys
		for ( usize i = 0; i < mPositionKeys.size( ); ++i )
		{
			// Grab reference to keyframe
			KeyFrame< Vec3 >& k = mPositionKeys.at( i ); 
			
			// Time stamp
			k.mTimeStamp = buffer->Read< f32 >( );

			// Vec3 position
			k.mValue.x = buffer->Read< f32 >( );
			k.mValue.y = buffer->Read< f32 >( );
			k.mValue.z = buffer->Read< f32 >( );
		}

		// Read in scale key size
		mScaleKeys.resize( buffer->Read< usize >( ) );

		// Write out rotation keys
		for ( usize i = 0; i < mScaleKeys.size( ); ++i )
		{
			// Grab reference to keyframe
			KeyFrame< Vec3 >& k = mScaleKeys.at( i ); 
			
			// Time stamp
			k.mTimeStamp = buffer->Read< f32 >( );

			// Vec3 scale
			k.mValue.x = buffer->Read< f32 >( );
			k.mValue.y = buffer->Read< f32 >( );
			k.mValue.z = buffer->Read< f32 >( );
		} 

		return Result::SUCCESS;
	}

	//=============================================================== 

	u32 ChannelData::GetRotationFrameID( const f32& time ) const
	{
		for ( u32 i = 1; i < mRotationKeys.size( ); ++i )
		{
			if ( time < mRotationKeys.at( i ).mTimeStamp )
			{
				return i - 1;
			}
		}

		return 0;
	}

	//=============================================================== 

	u32 ChannelData::GetPositionFrameID( const f32& time ) const
	{
		for ( u32 i = 1; i < mPositionKeys.size( ); ++i )
		{
			if ( time < mPositionKeys.at( i ).mTimeStamp )
			{
				return i - 1;
			}
		} 

		return 0;
	}

	//=============================================================== 
	
	u32 ChannelData::GetScaleFrameID( const f32& time ) const
	{ 
		for ( u32 i = 1; i < mScaleKeys.size( ); ++i )
		{
			if ( time < mScaleKeys.at( i ).mTimeStamp )
			{
				return i - 1;
			}
		} 

		return 0;
	} 

	//===============================================================

	f32 SkeletalAnimation::GetDuration( ) const
	{
		return mNumberOfTicks;
	}

	//===============================================================

	f32 SkeletalAnimation::GetTicksPerSecond( ) const
	{
		return mTicksPerSecond;
	}

	//===============================================================

	f32 Map01( const f32& min, const f32& max, const f32& t )
	{
		f32 range = std::abs( max - min );
		return ( t - min ) / ( range == 0.0f ? 0.00001f : range );
	}

	Transform SkeletalAnimation::CalculateInterpolatedTransform( const f32& time, const u32& boneID ) const
	{
		// Just return identity transform
		if ( boneID > mChannelData.size() || mChannelData.empty() )
		{ 
			return Transform( );
		}

		// Grab channel data for bone
		const ChannelData* data = &mChannelData.at( boneID );

		// Return default transform if empty channel data
		if ( data->mRotationKeys.empty( ) || data->mPositionKeys.empty( ) || data->mScaleKeys.empty( ) )
		{
			return Transform( );
		}

		// Need to get appropriate keyframe for data
		u32 rotationID = data->GetRotationFrameID( time );
		u32 positionID = data->GetPositionFrameID( time );
		u32 scaleID = data->GetScaleFrameID( time ); 

		u32 nextRotationID = ( rotationID + 1 ) % data->mRotationKeys.size( );
		u32 nextPositionID = ( positionID + 1 ) % data->mPositionKeys.size( );
		u32 nextScaleID = ( scaleID + 1 ) % data->mScaleKeys.size( );

		// This current frame's transform
		const Vec3& position = data->mPositionKeys.at( positionID ).mValue;
		const Quaternion& rotation = data->mRotationKeys.at( rotationID ).mValue;
		const Vec3& scale = data->mScaleKeys.at( scaleID ).mValue;

		// Current frame's Time stamps
		const f32& posTime = data->mPositionKeys.at( positionID ).mTimeStamp;
		const f32& rotTime = data->mPositionKeys.at( rotationID ).mTimeStamp;
		const f32& sclTime = data->mPositionKeys.at( scaleID ).mTimeStamp;

		// Next frame's transform
		const Vec3& nextPosition = data->mPositionKeys.at( nextPositionID ).mValue;
		const Quaternion& nextRotation = data->mRotationKeys.at( nextRotationID ).mValue;
		const Vec3& nextScale = data->mScaleKeys.at( nextScaleID ).mValue; 

		// Next frame's Time stamps
		const f32& nextPosTime = data->mPositionKeys.at( nextPositionID ).mTimeStamp;
		const f32& nextRotTime = data->mPositionKeys.at( nextRotationID ).mTimeStamp;
		const f32& nextSclTIme = data->mPositionKeys.at( nextScaleID ).mTimeStamp; 

		f32 posFactor = Map01( posTime, nextPosTime, time );
		f32 rotFactor = Map01( rotTime, nextRotTime, time );
		f32 sclFactor = Map01( sclTime, nextSclTIme, time );
 
		Vec3 lerpPos = Vec3::Lerp( position, nextPosition, posFactor );
		Quaternion lerpRot = Quaternion::Slerp( rotation, nextRotation, rotFactor ).Normalize( );
		Vec3 lerpScl = Vec3::Lerp( scale, nextScale, sclFactor );

		return Transform( lerpPos, lerpRot, lerpScl );
	}

	//=============================================================== 

	Result SkeletalAnimation::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Channel data size
		buffer->Write< usize >( mChannelData.size( ) );

		// Write out channel data
		for ( auto& c : mChannelData )
		{
			c.SerializeData( buffer );
		} 

		// Ticks per second
		buffer->Write< f32 >( mTicksPerSecond );

		// Number of ticks
		buffer->Write< f32 >( mNumberOfTicks ); 

		// Write out skeleton asset reference
		buffer->Write< UUID >( mSkeleton.GetUUID( ) );

		return Result::SUCCESS;
	}

	//=============================================================== 

	Result SkeletalAnimation::DeserializeData( ByteBuffer* buffer )
	{
		// Read in channel data size
		mChannelData.resize( buffer->Read< usize >( ) );

		// Read in channel data
		for ( usize i = 0; i < mChannelData.size( ); ++i )
		{ 
			// Grab reference to channel data
			ChannelData& c = mChannelData.at( i );

			c.DeserializeData( buffer );
		}

		// Ticks per second
		mTicksPerSecond = buffer->Read< f32 >( );

		// Number of ticks
		mNumberOfTicks = buffer->Read< f32 >( ); 

		// Set skeleton asset
		mSkeleton = EngineSubsystem( AssetManager )->GetAsset< Skeleton >( buffer->Read< UUID >( ) );

		return Result::SUCCESS; 
	}

	//=============================================================== 

	/*
	* @brief
	*/
	void SkeletalAnimation::CalculateTransform( const u32& jointID, const Mat4x4& parentMatrix, Vector<Mat4x4>& outMatrices, const f32& time ) const
	{
		const Skeleton* skeleton = mSkeleton.Get( ); 

		// Get joint
		const Joint* joint = &skeleton->mJoints.at( jointID );

		// Calculate bone transform ( bone space ) // Identity for now ( No animation )
		Mat4x4 jointTransform = Mat4x4::Identity( );
		//if ( animation )
		{
			Transform t = CalculateInterpolatedTransform( time, jointID );
			jointTransform = t.ToMat4x4( );
		}

		// Calculate relative to parent
		Mat4x4 globalTransform = parentMatrix * jointTransform;

		// Calculate and set local space matrix
		//if ( animation != nullptr )
		//{
		//	//outMatrices.at( joint->mID ) = mGlobalInverseTransform * relativeTransform * joint->mInverseBindMatrix; 
		//	//outMatrices.at( joint->mID ) = globalTransform * joint->mInverseBindMatrix; 
		//}
		outMatrices.at( joint->mID ) = skeleton->mGlobalInverseTransform * globalTransform * joint->mInverseBindMatrix;
		//else
		//{
		//	outMatrices.at( joint->mID ) = mGlobalInverseTransform * joint->mInverseBindMatrix;
		//}

		// Iterate through children 
		for ( u32 i = 0; i < joint->mChildren.size( ); ++i )
		{
			CalculateTransform( joint->mChildren.at( i ), globalTransform, outMatrices, time );
		}

	}

	//=============================================================== 
}
