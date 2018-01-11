// @file BoxCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/BoxCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	BoxCollisionShape::BoxCollisionShape( )
	{
		// Construct box collision shape
		mShape = new BulletBoxShape( BV3( mHalfExtents.x, mHalfExtents.y, mHalfExtents.z ) ); 

		// Set up shape type
		mShapeType = CollisionShapeType::Box;
	}

	//==============================================================

	BoxCollisionShape::~BoxCollisionShape( )
	{
		// Release memory for shape
		delete mShape;
		mShape = nullptr;
	}

	//==============================================================

	void BoxCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================
}

