// @file EmptyCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/EmptyCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	void EmptyCollisionShape::ExplicitConstructor( )
	{
		// Construct box collision shape
		mShape = new btEmptyShape( );

		// Set up shape type
		mShapeType = CollisionShapeType::Empty;
	}

	//============================================================== 

	void EmptyCollisionShape::ExplicitDestructor( )
	{
		// Release memory for shape
		delete mShape;
		mShape = nullptr;
	}

	//==============================================================

	void EmptyCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================
}
