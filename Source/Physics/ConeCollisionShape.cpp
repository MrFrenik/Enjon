// @file ConeCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/ConeCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	void ConeCollisionShape::ExplicitConstructor( )
	{
		// Construct box collision shape
		mShape = new BulletConeShape( mRadius, mHeight );

		// Set up shape type
		mShapeType = CollisionShapeType::Cone;
	} 

	//==============================================================

	void ConeCollisionShape::ExplicitDestructor( )
	{
		// Release memory for shape
		DeleteShape( );
	}

	//==============================================================

	void ConeCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================
}
