// @file AnimatoinSubsystem.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/AnimationSubsystem.h"
#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Entity/EntityManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//==========================================================================

	Result AnimationSubsystem::Initialize( )
	{
		return Result::SUCCESS;
	}

	//==========================================================================

	void AnimationSubsystem::Update( const f32 dT )
	{
		EntityManager* em = EngineSubsystem( EntityManager );
		Vector< Component* > comps = em->GetAllComponentsOfType< SkeletalAnimationComponent >( ); 
		for ( auto& c : comps )
		{
			// Cast to skeletal animation component
			SkeletalAnimationComponent* sac = c->ConstCast< SkeletalAnimationComponent >( );

			if ( sac )
			{
				// Calculate transforms for this frame
				sac->UpdateAndCalculateTransforms(); 
			} 
		}
	}

	//==========================================================================

	Result AnimationSubsystem::Shutdown( )
	{
		return Result::SUCCESS;
	}

	//==========================================================================
}

/*
	ENJON_CLASS( Construct )
	class SkeletalAnimationComponent : public Component
	{
		ENJON_COMPONENT_BODY( SkeletalAnimationComponent )

		public: 

			virtual void Update( ) override; 

		protected: 

			void UpdateAndCalculateTransforms();

		private:
			ENJON_PROPERTY( )
			AssetHandle< SkeletalAnimation > mAnimation; 

			f32 mCurrentTime = 0.0f;

			Vector< Mat4x4 > mJointTransforms; 
	}; 

	SkeletalAnimationComponent::UpdateAndCalculateTransforms()
	{
		// Do things here...  
	}

*/

