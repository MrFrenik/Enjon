#ifndef AABB_H
#define AABB_H

#include "Math/Maths.h"

namespace Enjon { namespace Physics {

	// AABB Collision struct
	typedef struct
	{
		Enjon::Math::Vec2 Min;
		Enjon::Math::Vec2 Max;
	} AABB;

	// Tests intersection between two AABBs
	bool AABBvsAABB(const AABB* A, const AABB* B);

	// Resolves intersection between two AABBs
	bool Resolve(AABB* A, AABB* B);

	// Calculates and returns Minimum Translation Distance of AABBvsAABB collision
	Enjon::Math::Vec2 MinimumTranslation(const AABB* A, const AABB* B);
		
}}

#endif