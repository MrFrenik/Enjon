// @file PhysicsSubsystem.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/PhysicsSubsystem.h" 
#include "Physics/CollisionShape.h"
#include "Physics/RigidBody.h"
#include "Entity/EntityManager.h"
#include "Entity/Components/RigidBodyComponent.h"
#include "Physics/PhysicsUtils.h"
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
		mCollisionConfiguration = new BulletDefaultCollisionConfiguration( );

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		mDispatcher = new BulletCollisionDispatcher( mCollisionConfiguration );

		///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
		mOverlappingPairCache = new BulletDBVTBroadPhase( );

		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		mSolver = new BulletSequentialImpulseConstraintSolver();

		// Set up physics world
		mDynamicsWorld = new BulletDiscreteDynamicWorld( mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration );

		// Set up physics world gravity
		mDynamicsWorld->setGravity( BV3( 0, -10, 0 ) ); 

		return Result::SUCCESS;
	}

	//======================================================================

	void PhysicsSubsystem::Update( const f32 dt )
	{
		// Set physics simulation
		if ( !mIsPaused )
		{
			WorldTime wt = Engine::GetInstance( )->GetWorldTime( );

			// Simulate the dynamics world step
			//mDynamicsWorld->stepSimulation( 1.f / 60.f, 0 ); 
			mDynamicsWorld->stepSimulation( dt * wt.GetTimeScale(), 10 ); 

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
			RigidBodyComponent* compA = (RigidBodyComponent* )( ( static_cast< const btCollisionObject* > ( contactManifold->getBody0( ) ) )->getUserPointer( ) );
			RigidBodyComponent* compB = (RigidBodyComponent* )( ( static_cast< const btCollisionObject* > ( contactManifold->getBody1( ) ) )->getUserPointer( ) );

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
					RigidBodyComponent* compT;
					if ( idA > idB )
					{
						compT = compA;
						compA = compB;
						compB = compT;
					}

					// If set doesn't exist, then make it
					if ( mNewContactEvents.find( compA ) == mNewContactEvents.end( ) )
					{
						mNewContactEvents[ compA ] = HashSet< RigidBodyComponent* >( );
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
						compA->OnCollisionEnter( CollisionReport( compA, compB ) );
						compB->OnCollisionEnter( CollisionReport( compB, compA ) );
					} 
				}

				// No need to process the other contact points once found
				if ( inContact )
				{
					break;
				}
			}
		} 

		// Process all current contacts. If any of these pairs is not in new contacts, then end collision event is triggered.
		for ( auto& curKey : mContactEvents )
		{ 
			// If key is not in new events
			if ( mNewContactEvents.find( curKey.first ) == mNewContactEvents.end( ) )
			{
				for ( auto& curVal : mContactEvents[ curKey.first ] )
				{ 
					// End contact for values
					curVal->OnCollisionExit( CollisionReport( curVal, curKey.first ) );
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
						curVal->OnCollisionExit( CollisionReport( curVal, curKey.first ) );
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

	void PhysicsSubsystem::RemoveFromContactEvents( RigidBodyComponent* comp )
	{
		// Remove key instances
		auto query = mContactEvents.find( comp );
		if ( query != mContactEvents.end( ) )
		{
			mContactEvents.erase( comp );
		} 

		// Now the tricky part; Have to iterate value instances for this comp as well
		for ( auto& k : mContactEvents )
		{
			k.second.erase( comp );
		}
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
		mDynamicsWorld->addRigidBody( body->GetRawBody() );
	}

	//====================================================================== 

	void PhysicsSubsystem::RemoveBody( RigidBody* body )
	{
		mRigidBodies.erase( body );
		mDynamicsWorld->removeRigidBody( body->GetRawBody() );
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
			b->ClearForces( );
		}
	}

	//====================================================================== 

	BulletDynamicPhysicsWorld* PhysicsSubsystem::GetWorld( ) const
	{
		return mDynamicsWorld;
	}

	//====================================================================== 

	void PhysicsSubsystem::CastRay( RayCastResult* ray )
	{
		if ( !ray )
		{
			return;
		} 

		BV3 start = PhysicsUtils::Vec3ToBV3( ray->mStart );
		BV3 end = PhysicsUtils::Vec3ToBV3( ray->mEnd );
		BulletClosestRayResultCallback callback( start, end );
		mDynamicsWorld->rayTest( PhysicsUtils::Vec3ToBV3( ray->mStart ), PhysicsUtils::Vec3ToBV3( ray->mEnd ), callback ); 

		if ( callback.hasHit( ) )
		{ 
			// Store user component pointer
			ray->mHitComponent = static_cast< RigidBodyComponent* >( callback.m_collisionObject->getUserPointer( ) );

			// Store hit point
			ray->mHitPoint = PhysicsUtils::BV3ToVec3( callback.m_hitPointWorld );

			// Store hit normal
			ray->mHitNormal = PhysicsUtils::BV3ToVec3( callback.m_hitNormalWorld ); 
		} 
	}

	//====================================================================== 

	Entity* CollisionReport::GetMatchingEntityFromReport( Component* comp ) const
	{
		if ( mCompA == nullptr || mCompB == nullptr || comp == nullptr )
		{
			return nullptr;
		}

		Entity* entA = mCompA->GetEntity( );
		Entity* entB = mCompB->GetEntity( );
		Entity* entC = comp->GetEntity( );

		Entity* retEnt = entA == entC ? entA : entB == entC ? entB : nullptr;
		return retEnt;
	}

	//====================================================================== 
			
	RayCastResult::RayCastResult( const Vec3& start, const Vec3& end )
		: mStart( start ), mEnd( end )
	{
	} 

	//====================================================================== 

	void RayCastResult::Cast( )
	{
		auto world = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< PhysicsSubsystem >( )->GetWorld( );

		BV3 start = PhysicsUtils::Vec3ToBV3( mStart );
		BV3 end = PhysicsUtils::Vec3ToBV3( mEnd );
		BulletClosestRayResultCallback callback( start, end );
		world->rayTest( PhysicsUtils::Vec3ToBV3( mStart ), PhysicsUtils::Vec3ToBV3( mEnd ), callback ); 

		if ( HasHit( ) )
		{
			// Store user component pointer
			mHitComponent = static_cast< RigidBodyComponent* >( callback.m_collisionObject->getUserPointer( ) );

			// Store hit point
			mHitPoint = PhysicsUtils::BV3ToVec3( callback.m_hitPointWorld );

			// Store hit normal
			mHitNormal = PhysicsUtils::BV3ToVec3( callback.m_hitNormalWorld ); 
		}
	}

	//====================================================================== 
}
