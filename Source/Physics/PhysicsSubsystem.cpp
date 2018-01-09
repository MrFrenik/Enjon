// @file PhysicsSubsystem.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/PhysicsSubsystem.h"

#include "Entity/EntityManager.h"
 
#include <Bullet/btBulletDynamicsCommon.h> 

#include <unordered_map>


// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
//btAlignedObjectArray<btCollisionShape*> collisionShapes;

namespace Enjon
{
	//======================================================================

	bool customContactAddedCallback( btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1 )
	{
		Entity* obj1 = reinterpret_cast< Entity* > ( colObj0Wrap->getCollisionObject( )->getUserPointer( ) );
		Entity* obj2 = reinterpret_cast< Entity* > ( colObj1Wrap->getCollisionObject( )->getUserPointer( ) );

		if ( obj1 == 0 || obj2 == 0 )
			return false;

		std::cout << "Begin contact!\n";

		return true;
	}

	bool customContactProcessedCallback( btManifoldPoint& cp, void* body0, void* body1 )
	{
		Entity* obj1 = reinterpret_cast< Entity* > ( static_cast< btCollisionObject* > ( body0 )->getUserPointer( ) );
		Entity* obj2 = reinterpret_cast< Entity* > ( static_cast< btCollisionObject* > ( body1 )->getUserPointer( ) );

		if ( obj1 == 0 || obj2 == 0 )
			return false; 



		std::cout << "Process contact!\n";

		return true;
	}

	bool customContactDestroyedCallback( void* ptr )
	{
		std::cout << "End contact!\n";

		return false;
	}

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

		//gContactAddedCallback = customContactAddedCallback;
		//gContactProcessedCallback = customContactProcessedCallback;
		//gContactDestroyedCallback = customContactDestroyedCallback;

		return Result::SUCCESS;
	}

	//======================================================================

	void PhysicsSubsystem::Update( const f32 dt )
	{
		// Set physics simulation
		if ( !mIsPaused )
		{
			// Simulate the dynamics world step
			mDynamicsWorld->stepSimulation( 1.f / 60.f, 10 ); 

			// Check collisions and process callbacks
			CheckCollisions( dt ); 
		}
	}

	//======================================================================

	void PhysicsSubsystem::CheckCollisions( const f32& dt )
	{
		std::unordered_multimap< u32, u32 > newContacts;
		//static HashMap< Entity*, Entity* > newContacts;
		//newContacts.clear( );
		
		// Browse all collision pairs.
		s32 numManifolds = mDynamicsWorld->getDispatcher( )->getNumManifolds( );
		for ( s32 i = 0; i < numManifolds; i++ )
		{

			btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher( )->getManifoldByIndexInternal( i );

			Entity* objA = ( Entity* )( ( static_cast< const btCollisionObject* > ( contactManifold->getBody0( ) ) )->getUserPointer( ) );
			Entity* objB = ( Entity* )( ( static_cast< const btCollisionObject* > ( contactManifold->getBody1( ) ) )->getUserPointer( ) ); 

			if ( objA == nullptr || objB == nullptr )
				return;

			u32 idA = objA->GetID( );
			u32 idB = objB->GetID( );

			// Check all contacts points.
			int numContacts = contactManifold->getNumContacts( );
			for ( int j = 0; j < numContacts; j++ )
			{
				btManifoldPoint& pt = contactManifold->getContactPoint( j );

				// Check collision state.
				if ( pt.getDistance( ) < 0.f )
				{
					// Order A < B, will make things easier with the contact hash.
					Entity* objT;
					if ( objA > objB )
					{
						objT = objA;
						objA = objB;
						objB = objT;
					}

					std::pair<u32, u32> pair( idA, idB );
					newContacts.insert( pair );

					// Call the collide() method on the bodies if they are not registered yet.
					bool contains = false;
					auto its = mContacts.equal_range( idA );
					for ( auto& it = its.first; it != its.second; ++it )
					{
						if ( it->second == idB )
						{
							contains = true;
							break; 
						}
					}

					if ( !contains )
					{
						//std::cout << "Begin contact!\n";
					} 
				}
			}
		}


		// If the contact pair is in mContacts but NOT in newContacts, then that contact pair is longer valid and must be removed
		bool found = false;
		for ( auto& ContactIter = mContacts.begin( ); ContactIter != mContacts.end( ); ++ContactIter )
		{
			// Find all keys that match this iterator
			auto NewContactKeyIter = newContacts.equal_range( ContactIter->first ); 

			for ( auto NewContactValueIter = NewContactKeyIter.first; NewContactValueIter != NewContactKeyIter.second; ++NewContactValueIter )
			{
				if ( NewContactValueIter->second == ContactIter->second )
				{
					found = true;
					break;
				}
			}
			if ( found )
			{
				break;
			}
		}

		if ( numManifolds && !found )
		{
			//std::cout << "End contact!\n";
		}
		if ( found )
		{
			//std::cout << "Process contact!\n";
		}


		mContacts = newContacts; 
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
