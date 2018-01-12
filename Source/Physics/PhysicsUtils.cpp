// @file PhysicsUtils.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/PhysicsDefines.h"
#include "Physics/PhysicsUtils.h"
#include "System/Types.h"
#include "Math/Maths.h"

#include <Bullet/btBulletCollisionCommon.h>

namespace Enjon 
{
	namespace PhysicsUtils
	{
		BV3 Vec3ToBV3( const Vec3& vec ) 
		{
			return BV3( vec.x, vec.y, vec.z );
		} 

		Vec3 BV3ToVec3( const BV3& vec )
		{
			return Vec3( vec.getX( ), vec.getY( ), vec.getZ( ) );
		}
	}
}
