// @file CollisionReport.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#ifndef ENJON_COLLISION_REPORT_H
#define ENJON_COLLISION_REPORT_H

#include "Entity/EntityManager.h"

namespace Enjon
{
	class RigidBodyComponent;
	class CollisionReport
	{ 
		public:

			CollisionReport( RigidBodyComponent* cmpA, RigidBodyComponent* cmpB )
				: mCompA( cmpA ), mCompB( cmpB )
			{
			}

			EntityHandle GetMatchingEntityFromReport( Component* comp ) const;
			EntityHandle GetNonMatchingEntityFromReport( Component* comp ) const;

			RigidBodyComponent* mCompA;
			RigidBodyComponent* mCompB;
	}; 
}

#endif
