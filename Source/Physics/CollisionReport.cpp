// @file CollisionReport.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/CollisionReport.h"
#include "Entity/Components/RigidBodyComponent.h"

namespace Enjon
{
	//====================================================================== 

	EntityHandle CollisionReport::GetMatchingEntityFromReport( Component* comp ) const
	{
		if ( mCompA == nullptr || mCompB == nullptr || comp == nullptr )
		{
			return nullptr;
		}

		Entity* entA = mCompA->GetEntity( );
		Entity* entB = mCompB->GetEntity( );
		Entity* entC = comp->GetEntity( );

		// Look for matching entity
		Entity* retEnt = entA == entC ? entA : entB == entC ? entB : nullptr;
		return EntityHandle( retEnt );
	}

	//====================================================================== 

	EntityHandle CollisionReport::GetNonMatchingEntityFromReport( Component* comp ) const
	{
		if ( mCompA == nullptr || mCompB == nullptr || comp == nullptr )
		{
			return nullptr;
		}

		Entity* entA = mCompA->GetEntity( );
		Entity* entB = mCompB->GetEntity( );
		Entity* entC = comp->GetEntity( );

		// Look for non-matching entity
		Entity* retEnt = entA == entC ? entB : entB == entC ? entA : nullptr;
		return EntityHandle( retEnt );
	}
}

