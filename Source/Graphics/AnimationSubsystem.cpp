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
				// Update animation transforms ( or however this needs to operate... )
				c->CalculateTransforms(); 
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

