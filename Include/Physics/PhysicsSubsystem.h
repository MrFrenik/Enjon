// @file PhysicsSubsystem.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_PHYSICS_SUBSYSTEM_H
#define ENJON_PHYSICS_SUBSYSTEM_H

#include "System/Types.h"
#include "Subsystem.h"
#include "Math/Vec3.h"

// TODO(): Abstract away Bullet Implementations
#include <Bullet/btBulletCollisionCommon.h> 
#include <Bullet/btBulletDynamicsCommon.h>

using RigidBody				= btRigidBody;
using CollisionShape		= btCollisionShape;
using RigidBodyMotionState	= btMotionState;
using PhysicsWorld			= btDynamicsWorld;

namespace Enjon 
{ 
	class Entity; 

	class RayCastResult
	{ 
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
			void ClearAllForces( );

			/**
			*@brief
			*/
			PhysicsWorld* GetWorld( ) const;

		private:
			/**
			*@brief
			*/
			void PhysicsSubsystem::CheckCollisions( const f32& dt );

		private:
			btDiscreteDynamicsWorld* mDynamicsWorld						= nullptr;
			btDefaultCollisionConfiguration* mCollisionConfiguration	= nullptr; 
			btCollisionDispatcher* mDispatcher							= nullptr; 
			btBroadphaseInterface* mOverlappingPairCache				= nullptr; 
			btSequentialImpulseConstraintSolver* mSolver				= nullptr;

			HashSet<RigidBody*> mRigidBodies;
			HashMap< u32, HashSet< u32 > > mContactEvents; 
			HashMap< u32, HashSet< u32 > > mNewContactEvents; 

			u32 mIsPaused = false;
	}; 
}

#endif
