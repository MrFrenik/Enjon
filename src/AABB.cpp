#include "Physics/AABB.h"

namespace Enjon { namespace Physics {

	// Tests intersection between two AABBs
	bool AABBvsAABB(const AABB* A, const AABB* B)
	{
		if (B->Max.x < A->Min.x || 
			B->Max.y < A->Min.y ||
			B->Min.x > A->Max.x ||
			B->Min.y > A->Max.y )
		{
			return false;
		}

		return true;
	}

	// Resolves intersection between two AABBs
	bool Resolve(AABB* A, AABB* B)
	{
		return 0;
	}

	// Calculates and returns Minimum Translation Distance of AABBvsAABB collision
	Enjon::Math::Vec2 MinimumTranslation(const AABB* A, const AABB* B)
	{
		Enjon::Math::Vec2 MTD;

		float left		= B->Min.x - A->Max.x;
		float right		= B->Max.x - A->Min.x;
		float top		= B->Min.y - A->Max.y;
		float bottom	= B->Max.y - A->Min.y;

		if (abs(left) < right) 
			MTD.x = left;
		else 
			MTD.x = right;

		if (abs(top) < bottom) 
			MTD.y = top;
		else 
			MTD.y = bottom;

		if (abs(MTD.x) < abs(MTD.y)) 
			MTD.y = 0;
		else
			MTD.x = 0;

		return MTD;
	}

}}