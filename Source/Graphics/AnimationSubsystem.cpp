// @file AnimatoinSubsystem.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/AnimationSubsystem.h"
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
		/*
			EntityManager* em = EngineSubsystem( EntityManager );
			Vector< SkeletalAnimationComponent* > comps = em->GetAllComponentsOfType< SkeletalAnimationComponent >( ); 
			for ( auto& c : comps )
			{
				// Calculate transforms for this frame
				c->CalculateTransforms(); 

				// Update animation time for next frame
				c->mCurrentTime += Engine::GetInstance()->GetWorldTime()->GetDeltaTime();
			}
		*/
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

			void CalculateTransforms();

		private:
			ENJON_PROPERTY( )
			AssetHandle< SkeletalAnimation > mAnimation; 

			f32 mCurrentTime = 0.0f;

			Vector< Mat4x4 > mJointTransforms; 
	}; 

	SkeletalAnimationComponent::CalculateTransforms()
	{
		// Do things here...  
	}

*/

