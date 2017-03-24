#ifndef ENJON_AABB_H
#define ENJON_AABB_H

#include "Math/Maths.h"
#include "Defines.h"

namespace Enjon { namespace Physics {

	// AABB Collision struct
	typedef struct
	{
		Enjon::Vec2 Min;
		Enjon::Vec2 Max;
	} AABB;

	// Test point vs AABB
	bool AABBvsPoint(const AABB* A, const Enjon::Vec2& P);

	// Tests intersection between two AABBs
	bool AABBvsAABB(const AABB* A, const AABB* B);

	// Resolves intersection between two AABBs
	bool Resolve(AABB* A, AABB* B);

	// Calculates and returns Minimum Translation Distance of AABBvsAABB collision
	Enjon::Vec2 MinimumTranslation(const AABB* A, const AABB* B);
		
}}

#endif