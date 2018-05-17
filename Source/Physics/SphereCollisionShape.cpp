// @file SphereCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/SphereCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	void SphereCollisionShape::ExplicitConstructor( )
	{
		// Construct box collision shape
		mShape = new BulletSphereShape( mRadius );

		// Set up shape type
		mShapeType = CollisionShapeType::Sphere;
	}

	//==============================================================

	SphereCollisionShape::SphereCollisionShape( RigidBody* body )
		: CollisionShape( body )
	{ 
		// Construct box collision shape
		mShape = new BulletSphereShape( mRadius );

		// Set up shape type
		mShapeType = CollisionShapeType::Sphere;
	}

	//==============================================================

	void SphereCollisionShape::ExplicitDestructor( )
	{
		// Release memory for shape
		DeleteShape( );
	}

	//==============================================================

	void SphereCollisionShape::Base( )
	{
		// Does nothing...
	}

	//==============================================================
}

