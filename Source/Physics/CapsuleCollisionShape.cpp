// @file CapsuleCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/CapsuleCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	CapsuleCollisionShape::CapsuleCollisionShape( )
	{
		// Construct box collision shape
		mShape = new BulletCapsuleShape( mRadius, mHeight );

		// Set up shape type
		mShapeType = CollisionShapeType::Capsule;
	}

	//==============================================================

	CapsuleCollisionShape::CapsuleCollisionShape( RigidBody* body )
		: CollisionShape( body )
	{
		// Construct box collision shape
		mShape = new BulletCapsuleShape( mRadius, mHeight );

		// Set up shape type
		mShapeType = CollisionShapeType::Capsule; 
	}

	//==============================================================

	CapsuleCollisionShape::~CapsuleCollisionShape( )
	{
		// Release memory for shape
		DeleteShape( );
	}

	//==============================================================

	void CapsuleCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================
}
