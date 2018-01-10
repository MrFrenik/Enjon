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

namespace Enjon 
{ 
	class Entity;

	struct PhysicsForceWrapper
	{
		Vec3 mForce;
		Vec3 mRelativePosition;
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

			btDynamicsWorld* GetWorld( )
			{
				return mDynamicsWorld;
			}

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

			std::unordered_multimap< u32, u32 > mContacts;
			HashSet<RigidBody*> mRigidBodies;

			u32 mIsPaused = false;
	}; 
}

#endif
