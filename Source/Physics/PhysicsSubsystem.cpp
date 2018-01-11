// @file PhysicsSubsystem.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/PhysicsSubsystem.h" 
#include "Entity/EntityManager.h"
#include "Entity/Components/PhysicsComponent.h"
#include "SubsystemCatalog.h"
#include "Engine.h"
 
#include <Bullet/btBulletDynamicsCommon.h> 

#include <unordered_map>


// Keep track of all bullet shapes
// Make sure to reuse shapes amongst rigid bodies whenever possible
//btAlignedObjectArray<btCollisionShape*> collisionShapes;

namespace Enjon
{ 
	//=====================================================================

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
		// Browse all collision pairs.
		s32 numManifolds = mDynamicsWorld->getDispatcher( )->getNumManifolds( );
		for ( s32 i = 0; i < numManifolds; i++ )
		{
			btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher( )->getManifoldByIndexInternal( i );

			// Get user pointers as components
			PhysicsComponent* compA = ( PhysicsComponent* )( ( static_cast< const btCollisionObject* > ( contactManifold->getBody0( ) ) )->getUserPointer( ) );
			PhysicsComponent* compB = ( PhysicsComponent* )( ( static_cast< const btCollisionObject* > ( contactManifold->getBody1( ) ) )->getUserPointer( ) ); 

			if ( compA == nullptr || compB == nullptr )
			{
				return; 
			}

			Entity* entA = compA->GetEntity( );
			Entity* entB = compB->GetEntity( );

			if ( entA == nullptr || entB == nullptr )
			{
				return;
			}

			// Get entity ids
			u32 idA = entA->GetID( );
			u32 idB = entB->GetID( );

			// Check all contacts points.
			int numContacts = contactManifold->getNumContacts( );
			bool inContact = false;
			for ( int j = 0; j < numContacts; j++ )
			{
				btManifoldPoint& pt = contactManifold->getContactPoint( j );

				// Check collision state.
				if ( pt.getDistance( ) < 0.f )
				{
					inContact = true;

					// Order A < B, will make things easier with the contact hash.
					PhysicsComponent* compT;
					if ( idA > idB )
					{
						compT = compA;
						compA = compB;
						compB = compT;
					}

					// If set doesn't exist, then make it
					if ( mNewContactEvents.find( compA ) == mNewContactEvents.end( ) )
					{
						mNewContactEvents[ compA ] = HashSet< PhysicsComponent* >( ); 
					}

					// Insert new contact event into set for idA
					mNewContactEvents[ compA ].insert( compB );

					// Now need to check if this collision exist in current contacts. If not, then collision begin event has begun.  
					bool contains = false;
					if ( mContactEvents.find( compA ) != mContactEvents.end() )
					{
						if ( mContactEvents[ compA ].find( compB ) != mContactEvents[ compA ].end( ) )
						{
							contains = true;
						}
					}

					// Was not there, so we have begun contact
					if ( !contains )
					{
						compA->OnCollisionEnter( CollisionReport( compB ) );
						compB->OnCollisionEnter( CollisionReport( compA ) );
					} 
				}

				// No need to process the other contact points once found
				if ( inContact )
				{
					break;
				}
			}
		} 

		EntityManager* entities = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< EntityManager >( )->ConstCast< EntityManager >();

		// Process all current contacts. If any of these pairs is not in new contacts, then end collision event is triggered.
		for ( auto& curKey : mContactEvents )
		{ 
			// If key is not in new events
			if ( mNewContactEvents.find( curKey.first ) == mNewContactEvents.end( ) )
			{
				for ( auto& curVal : mContactEvents[ curKey.first ] )
				{ 
					// End contact for values
					curVal->OnCollisionExit( CollisionReport( curKey.first ) );
				}
			}
			else
			{
				auto newSet = &mNewContactEvents[ curKey.first ];

				// Else the key is found. Now need to make sure that the pairs are found as well
				for ( auto& curVal : mContactEvents[ curKey.first ] )
				{
					// Not found in the newer set, so must process end contact event
					if ( newSet->find( curVal ) == newSet->end( ) )
					{ 
						curVal->OnCollisionExit( CollisionReport( curKey.first ) );
					}
					// Otherwise was found, so processing current contact
					else
					{ 
						//	std::cout << "Processing Contact!\n"; 
					}
				}
			}
		} 

		// Set current contact events to the newly found/merged ones
		mContactEvents = mNewContactEvents; 
		mNewContactEvents.clear( );
	}

	//======================================================================

	Result PhysicsSubsystem::Shutdown( )
	{
		return Result::SUCCESS;
	}

	//====================================================================== 

	void PhysicsSubsystem::Reset( )
	{
		mContactEvents.clear( ); 
		mNewContactEvents.clear( );
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

	PhysicsWorld* PhysicsSubsystem::GetWorld( ) const
	{
		return mDynamicsWorld;
	}

	//====================================================================== 
}
