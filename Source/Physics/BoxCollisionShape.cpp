// @file BoxCollisionShape.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/BoxCollisionShape.h"
#include "Physics/PhysicsUtils.h"

namespace Enjon
{
	//==============================================================

	void BoxCollisionShape::ExplicitConstructor( )
	{
		// Construct box collision shape
		mShape = new BulletBoxShape( BV3( mHalfExtents.x, mHalfExtents.y, mHalfExtents.z ) ); 

		// Set up shape type
		mShapeType = CollisionShapeType::Box;
	}

	//==============================================================

	BoxCollisionShape::BoxCollisionShape( const ResourceHandle< RigidBody >& body )
		: CollisionShape( body )
	{ 
		// Construct box collision shape
		mShape = new BulletBoxShape( BV3( mHalfExtents.x, mHalfExtents.y, mHalfExtents.z ) ); 

		// Set up shape type
		mShapeType = CollisionShapeType::Box;
	}
	
	//==============================================================

	void BoxCollisionShape::ExplicitDestructor( )
	{
		// Release memory for shape
		DeleteShape( );
	}

	//==============================================================

	void BoxCollisionShape::Base( )
	{ 
		// Does nothing...
	}

	//==============================================================

	void BoxCollisionShape::ConstructShape( )
	{
		// If shape, then delete previous
		if ( mShape )
		{
			DeleteShape( );
		}

		// Construct shape using half extents
		mShape = new BulletBoxShape( BV3( mHalfExtents.x, mHalfExtents.y, mHalfExtents.z ) ); 
	}

	//==============================================================

	void BoxCollisionShape::SetHalfExtents( const Vec3& extents )
	{
		// Set half extents
		mHalfExtents = extents; 

		// Free memory of previous shape
		DeleteShape( );

		// Construct shape
		ConstructShape( ); 
	}

	//==============================================================
}

