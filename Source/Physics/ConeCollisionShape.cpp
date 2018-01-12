// @file ConeCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/ConeCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	ConeCollisionShape::ConeCollisionShape( )
	{
		// Construct box collision shape
		mShape = new BulletConeShape( mRadius, mHeight );

		// Set up shape type
		mShapeType = CollisionShapeType::Cone;
	}

	//==============================================================

	ConeCollisionShape::~ConeCollisionShape( )
	{
		// Release memory for shape
		delete mShape;
		mShape = nullptr;
	}

	//==============================================================

	void ConeCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================
}
