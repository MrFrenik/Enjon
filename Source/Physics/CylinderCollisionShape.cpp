// @file CylinderCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/CylinderCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	CylinderCollisionShape::CylinderCollisionShape( )
	{
		// Construct box collision shape
		mShape = new BulletCylinderShape( BV3( mHalfExtents.x, mHalfExtents.y, mHalfExtents.z ) );

		// Set up shape type
		mShapeType = CollisionShapeType::Cylinder;
	}

	//==============================================================

	CylinderCollisionShape::~CylinderCollisionShape( )
	{
		// Release memory for shape
		delete mShape;
		mShape = nullptr;
	}

	//==============================================================

	void CylinderCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================
}
