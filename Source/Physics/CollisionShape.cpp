// @file CollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/CollisionShape.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsSubsystem.h"
#include "Physics/RigidBody.h"
#include "Entity/Components/RigidBodyComponent.h"
#include "Math/Vec3.h"

namespace Enjon
{ 
	//===========================================================

	CollisionShape::CollisionShape( )
	{ 
	}

	//===========================================================

	CollisionShape::~CollisionShape( )
	{ 
		// Delete the shape
		if ( mShape )
		{
			delete mShape;
			mShape = nullptr;
		}
	}

	//===========================================================

	BV3 CollisionShape::CalculateLocalInertia( const f32& mass )
	{ 
		BV3 localInertia( 0.0f, 0.0f, 0.0f );
		if ( mass != 0.0f && mShapeType != CollisionShapeType::Empty )
		{
			mShape->calculateLocalInertia( mass, localInertia );
		} 
		return localInertia;
	}

	//===========================================================

	BulletCollisionShape* CollisionShape::GetRawShape( ) const
	{
		return mShape;
	}

	//===========================================================

	void CollisionShape::SetLocalScaling( const Vec3& scale )
	{
		mLocalScaling = scale;
		mShape->setLocalScaling( PhysicsUtils::Vec3ToBV3( scale ) );
	}

	//===========================================================

	Vec3 CollisionShape::GetLocalScaling( )
	{
		return mLocalScaling;
	}

	//===========================================================

	CollisionShapeType CollisionShape::GetCollisionShapeType( ) const
	{
		return mShapeType;
	}

	//===========================================================
}
