// @file PhysicsSubsystem.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_PHYSICS_SUBSYSTEM_H
#define ENJON_PHYSICS_SUBSYSTEM_H

#include "System/Types.h"
#include "Subsystem.h"
#include "Math/Vec3.h" 
#include "Physics/PhysicsDefines.h"
#include "Entity/Component.h"
#include "Physics/RigidBody.h"
#include "Entity/Components/RigidBodyComponent.h"

namespace Enjon 
{ 
	class CollisionShape;
	class RigidBody;
	class RigidBodyComponent;
	class Entity; 
	class Component;

	class PhysicsDebugDrawer : public BulletIDebugDraw
	{
		public:
			/**
			* @brief
			*/
			PhysicsDebugDrawer( ) = default;

			/**
			* @brief
			*/
			virtual void drawLine( const BV3& from, const BV3& to, const BV3& color ); 

			/**
			* @brief
			*/
			virtual void drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color );

			/**
			* @brief
			*/
			virtual void reportErrorWarning( const char* warningString );

			/**
			* @brief
			*/
			virtual void draw3dText( const btVector3& location, const char* textString );

			/**
			* @brief
			*/
			virtual void setDebugMode( s32 debugMode );

			/**
			* @brief
			*/
			virtual s32 getDebugMode( ) const;

		private:
			s32 mDebugMode = 1;
	};

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

	ENJON_CLASS( ) 
	class PhysicsSubsystem : public Subsystem
	{ 
		ENJON_CLASS_BODY( PhysicsSubsystem )

		friend RigidBodyComponent;

		public: 

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
			virtual Result Shutdown() override;

			/**
			*@brief
			*/
			void AddBody( BulletRigidBody* body );
			//void AddBody( const ResourceHandle< RigidBody >& body ); 

			/**
			*@brief
			*/
			//void RemoveBody( const ResourceHandle< RigidBody >& body );
			void RemoveBody( BulletRigidBody* body );

			/**
			*@brief
			*/
			void PauseSystem( bool enabled );

			/**
			*@brief
			*/
			bool IsPaused( ) const;

			/**
			*@brief
			*/
			void RemoveFromContactEvents( ComponentHandle< RigidBodyComponent >* comp );

			/**
			*@brief
			*/
			void ClearAllForces( ); 

			/**
			*@brief
			*/
			const HashSet< ComponentHandle< RigidBodyComponent >* >* GetContactList( ComponentHandle< RigidBodyComponent >* component );

			/**
			*@brief
			*/
			BulletDynamicPhysicsWorld* GetWorld( ) const;

			/**
			*@brief
			*/
			void Reset( );

			/**
			*@brief
			*/
			void CastRay( RayCastResult* ray );

		protected:

			ResourceHandle< RigidBody > AllocateRigidBodyHandle( );

		private:

			/**
			*@brief
			*/
			bool HasContact( ComponentHandle< RigidBodyComponent >* component );

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
			HashMap< ComponentHandle< RigidBodyComponent >*, HashSet< ComponentHandle< RigidBodyComponent >* > > mContactEvents;
			HashMap< ComponentHandle< RigidBodyComponent >*, HashSet< ComponentHandle< RigidBodyComponent >* > > mNewContactEvents;

			PhysicsDebugDrawer mDebugDrawer;

			SlotArray< RigidBody > mRigidBodySlotArray;

			u32 mIsPaused = false;
			u32 mWasTicked = false;
	}; 
}

#endif
