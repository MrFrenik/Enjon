// @file PhysicsSubsystem.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_PHYSICS_SUBSYSTEM_H
#define ENJON_PHYSICS_SUBSYSTEM_H

#include "System/Types.h"
#include "Subsystem.h"
#include "Math/Vec3.h" 
#include "Physics/PhysicsDefines.h"

namespace Enjon 
{ 
	class CollisionShape;
	class RigidBody;
	class RigidBodyComponent;
	class Entity; 
	class Component;

	// What does a ray cast result give you?

	class RayCastResult
	{
		public: 

			/**
			* @brief
			*/
			RayCastResult( const Vec3& start, const Vec3& end );

			/**
			* @brief
			*/
			~RayCastResult( ) = default; 

			/**
			* @brief
			*/
			void Cast( );

			/**
			* @brief
			*/
			bool HasHit( )
			{
				return mHitComponent != nullptr;
			}

			/**
			* @brief
			*/
			void ClearResults( )
			{
				mHitComponent = nullptr;
			}

		public:
			Vec3 mStart;
			Vec3 mEnd;
			Vec3 mHitPoint;
			Vec3 mHitNormal;
			RigidBodyComponent* mHitComponent = nullptr; 
	};

	struct CollisionReport
	{ 
		CollisionReport( RigidBodyComponent* cmpA, RigidBodyComponent* cmpB )
			: mCompA( cmpA ), mCompB( cmpB )
		{
		}

		Entity* GetMatchingEntityFromReport( Component* comp ) const;

		RigidBodyComponent* mCompA;
		RigidBodyComponent* mCompB;
	};

	

	ENJON_CLASS( ) 
	class PhysicsSubsystem : public Subsystem
	{ 
		ENJON_CLASS_BODY( )

		public:

			/**
			*@brief
			*/
			PhysicsSubsystem( ) = default;

			/**
			*@brief
			*/
			~PhysicsSubsystem( ) = default;

			/**
			*@brief
			*/
			virtual Result Initialize();

			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) override;

			/**
			*@brief
			*/
			virtual Result Shutdown();

			/**
			*@brief
			*/
			void AddBody( RigidBody* body ); 

			/**
			*@brief
			*/
			void RemoveBody( RigidBody* body );

			/**
			*@brief
			*/
			void PauseSystem( bool enabled );

			/**
			*@brief
			*/
			void RemoveFromContactEvents( RigidBodyComponent* comp );

			/**
			*@brief
			*/
			void ClearAllForces( ); 

			/**
			*@brief
			*/
			BulletDynamicPhysicsWorld* GetWorld( ) const;

			/**
			*@brief
			*/
			void Reset( );

		private:
			/**
			*@brief
			*/
			void PhysicsSubsystem::CheckCollisions( const f32& dt );

		private:
			BulletDynamicPhysicsWorld* mDynamicsWorld						= nullptr;
			BulletDefaultCollisionConfiguration* mCollisionConfiguration	= nullptr;
			BulletCollisionDispatcher* mDispatcher							= nullptr;
			BulletBroadphaseInterface* mOverlappingPairCache				= nullptr; 
			BulletSequentialImpulseConstraintSolver* mSolver				= nullptr;

			HashSet<RigidBody*> mRigidBodies;
			HashMap< RigidBodyComponent*, HashSet< RigidBodyComponent* > > mContactEvents;
			HashMap< RigidBodyComponent*, HashSet< RigidBodyComponent* > > mNewContactEvents;

			u32 mIsPaused = false;
	}; 
}

#endif
