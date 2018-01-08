// @file PhysicsSubsystem.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/PhysicsSubsystem.h"
 
#include <Bullet/btBulletDynamicsCommon.h> 


// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
//btAlignedObjectArray<btCollisionShape*> collisionShapes;

namespace Enjon
{
	//======================================================================

	Result PhysicsSubsystem::Initialize( )
	{
		///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
		mCollisionConfiguration = new btDefaultCollisionConfiguration( );

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		mDispatcher = new btCollisionDispatcher( mCollisionConfiguration );

		///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
		mOverlappingPairCache = new btDbvtBroadphase( );

		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		mSolver = new btSequentialImpulseConstraintSolver;

		// Set up physics world
		mDynamicsWorld = new btDiscreteDynamicsWorld( mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration );

		// Set up physics world gravity
		mDynamicsWorld->setGravity( btVector3( 0, -10, 0 ) );

		//btCollisionShape* colShape = new btSphereShape(btScalar(1.0f)); 

		return Result::SUCCESS;
	}

	//======================================================================

	void PhysicsSubsystem::Update( const f32 dt )
	{
		// Set physics simulation
		if ( !mIsPaused )
		{
			mDynamicsWorld->stepSimulation( 1.f / 60.f, 10 ); 
		}
	}

	//======================================================================

	Result PhysicsSubsystem::Shutdown( )
	{
		return Result::SUCCESS;
	}

	//====================================================================== 

	void PhysicsSubsystem::AddBody( RigidBody* body )
	{
		mRigidBodies.insert( body );
		mDynamicsWorld->addRigidBody( body );
	}

	//====================================================================== 

	void PhysicsSubsystem::RemoveBody( RigidBody* body ) 
	{
		mRigidBodies.erase( body );
		mDynamicsWorld->removeRigidBody( body );
	}

	//====================================================================== 

	void PhysicsSubsystem::PauseSystem( bool enabled )
	{
		mIsPaused = ( u32 )enabled;
	}

	//====================================================================== 

	void PhysicsSubsystem::ClearAllForces( )
	{
		for ( auto& b : mRigidBodies )
		{
			b->clearForces( );
		}
	}

	//====================================================================== 
}
