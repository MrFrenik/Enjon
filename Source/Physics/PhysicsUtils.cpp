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

		//============================================================

		BV3 Vec3ToBV3( const Vec3& vec ) 
		{
			return BV3( vec.x, vec.y, vec.z );
		} 

		//============================================================

		BV3 iVec3ToBV3( const iVec3& vec )
		{
			return BV3( vec.x, vec.y, vec.z );
		 }

		//============================================================

		Vec3 BV3ToVec3( const BV3& vec )
		{
			return Vec3( vec.getX( ), vec.getY( ), vec.getZ( ) );
		}

		//============================================================

		Transform BTransformToTransform( const BTransform& transform )
		{ 
			// Create new Enjon Transform
			Transform newTrans;
			BV3 pos = transform.getOrigin( );
			BQuat rot = transform.getRotation( );

			newTrans.SetPosition( BV3ToVec3( pos ) );
			newTrans.SetRotation( Quaternion( rot.getX( ), rot.getY( ), rot.getZ( ), -rot.getW( ) ) ); 

			return newTrans;
		}

		//============================================================

		BTransform TransformToBTransform( const Transform& transform )
		{ 
			// Create new bullet transform
			BTransform bTransform;
			bTransform.setIdentity( );
			const Vec3* elp = &transform.GetPosition();
			const Quaternion* elr = &transform.GetRotation();
			const Vec3* els = &transform.GetScale();
			bTransform.setOrigin( BV3( elp->x, elp->y, elp->z ) );
			bTransform.setRotation( BQuat( elr->x, elr->y, elr->z, -elr->w ) );

			return bTransform;
		}

		//============================================================
	}
}
