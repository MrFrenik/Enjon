// @file SkeletalAnimationComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Entity/Components/SkeletalMeshComponent.h"
#include "Graphics/AnimationSubsystem.h"
#include "Entity/EntityManager.h"

namespace Enjon
{ 
	//==================================================================================

	void SkeletalAnimationComponent::UpdateAndCalculateTransforms( )
	{
		// Get skeletal mesh component and renderable from entity
		ComponentHandle< SkeletalMeshComponent > smc = mEntity->GetComponent< SkeletalMeshComponent >( );

		// If not valid, return ( NOTE(): this branching check will eventually go away with Require tags )
		if ( !smc )
		{
			return;
		}

		// Grab renderable from skeletal mesh component
		SkeletalMeshRenderable* renderable = &smc->mRenderable; 
		AssetHandle< Skeleton > skeleton = renderable->GetSkeleton( );

		if ( !skeleton || !mAnimation )
		{
			return;
		}

		// Make sure that skeletons line up between renderable and animation
		if ( skeleton.Get( ) != mAnimation.Get( )->mSkeleton.Get() )
		{
			return;
		}

		// If not valid skeleton, then return
		s32 rootID = skeleton.Get( )->GetRootID( );
		if ( !skeleton.Get( )->GetNumberOfJoints( ) || rootID == -1 )
		{
			return;
		}

		// Grab matrices to affect from renderable
		Vector< Mat4x4 >& mats = renderable->mFinalJointTransforms; 

		// Resize transforms necessary
		if ( mats.size( ) != skeleton.Get( )->GetNumberOfJoints( ) )
		{
			mats.resize( skeleton.Get( )->GetNumberOfJoints( ) );
		} 
		
		// Calculate matrix transforms for skeletal mesh
		skeleton.Get( )->CalculateTransform( rootID, Mat4x4::Identity( ), mats, mAnimation.Get(), mCurrentAnimationTime ); 

		// Increment current animation time ( this is hacked to just use a single animation for now... )
		mCurrentAnimationTime = std::fmod( mCurrentAnimationTime + Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( ) * mAnimationSpeed, mAnimation->GetDuration( ) );
	} 

	//================================================================================== 

	void SkeletalAnimationComponent::SetAnimation( const AssetHandle< SkeletalAnimation >& animation )
	{
		mAnimation = animation;
	}

	//================================================================================== 
}

