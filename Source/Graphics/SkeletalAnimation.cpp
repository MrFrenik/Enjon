// @file SkeletalAnimation.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/SkeletalAnimation.h"
#include "Asset/SkeletalAnimationAssetLoader.h"
#include "Graphics/Skeleton.h"

namespace Enjon
{ 
	//=============================================================== 

	u32 ChannelData::GetRotationFrameID( const f32& time )
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

	u32 ChannelData::GetPositionFrameID( const f32& time )
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
	
	u32 ChannelData::GetScaleFrameID( const f32& time )
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

	SkeletalAnimation::SkeletalAnimation( )
	{ 
	}

	//===============================================================

	SkeletalAnimation::~SkeletalAnimation( )
	{ 
	} 

	//===============================================================

	f32 SkeletalAnimation::GetDuration( ) const
	{
		return ( mNumberOfTicks / mTicksPerSecond );
	}

	f32 Map01( const f32& min, const f32& max, const f32& t )
	{
		f32 range = std::abs( max - min );
		return ( t - min ) / ( range == 0.0f ? 0.00001f : range );
	}

	Transform SkeletalAnimation::CalculateInterpolatedTransform( const f32& time, const u32& boneID )
	{
		// Just return identity transform
		if ( boneID > mChannelData.size() )
		{ 
			return Transform( );
		}

		// Grab channel data for bone
		ChannelData* data = &mChannelData.at( boneID );

		// Need to get appropriate keyframe for data
		u32 rotationID = data->GetRotationFrameID( time );
		u32 positionID = data->GetPositionFrameID( time );
		u32 scaleID = data->GetScaleFrameID( time ); 

		u32 nextRotationID = ( rotationID + 1 ) % data->mRotationKeys.size( );
		u32 nextPositionID = ( positionID + 1 ) % data->mPositionKeys.size( );
		u32 nextScaleID = ( scaleID + 1 ) % data->mScaleKeys.size( );

		// This current frame's transform
		Vec3& position = data->mPositionKeys.at( positionID ).mValue;
		Quaternion& rotation = data->mRotationKeys.at( rotationID ).mValue;
		Vec3& scale = data->mScaleKeys.at( scaleID ).mValue;

		// Current frame's Time stamps
		f32& posTime = data->mPositionKeys.at( positionID ).mTimeStamp;
		f32& rotTime = data->mPositionKeys.at( rotationID ).mTimeStamp;
		f32& sclTime = data->mPositionKeys.at( scaleID ).mTimeStamp;

		// Next frame's transform
		Vec3& nextPosition = data->mPositionKeys.at( nextPositionID ).mValue;
		Quaternion& nextRotation = data->mRotationKeys.at( nextRotationID ).mValue;
		Vec3& nextScale = data->mScaleKeys.at( nextScaleID ).mValue; 

		// Next frame's Time stamps
		f32& nextPosTime = data->mPositionKeys.at( nextPositionID ).mTimeStamp;
		f32& nextRotTime = data->mPositionKeys.at( nextRotationID ).mTimeStamp;
		f32& nextSclTIme = data->mPositionKeys.at( nextScaleID ).mTimeStamp; 

		f32 posFactor = Map01( posTime, nextPosTime, time );
		f32 rotFactor = Map01( rotTime, nextRotTime, time );
		f32 sclFactor = Map01( sclTime, nextSclTIme, time );
 
		Vec3 lerpPos = Vec3::Lerp( position, nextPosition, posFactor );
		//Vec3 lerpPos = position;
		Quaternion lerpRot = Quaternion::Slerp( rotation, nextRotation, rotFactor ).Normalize( );
		//Quaternion lerpRot = rotation;
		Vec3 lerpScl = Vec3::Lerp( scale, nextScale, sclFactor );

		return Transform( lerpPos, lerpRot, lerpScl );
	}

	//=============================================================== 
}
