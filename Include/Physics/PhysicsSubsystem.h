// @file PhysicsSubsystem.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_PHYSICS_SUBSYSTEM_H
#define ENJON_PHYSICS_SUBSYSTEM_H

#include "System/Types.h"
#include "Subsystem.h"

// TODO(): Abstract away Bullet Implementations
#include <Bullet/btBulletCollisionCommon.h> 
#include <Bullet/btBulletDynamicsCommon.h>

using RigidBody				= btRigidBody;
using CollisionShape		= btCollisionShape;
using RigidBodyMotionState	= btMotionState;

namespace Enjon 
{ 

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

		private:
			btDiscreteDynamicsWorld* mDynamicsWorld						= nullptr;
			btDefaultCollisionConfiguration* mCollisionConfiguration	= nullptr; 
			btCollisionDispatcher* mDispatcher							= nullptr; 
			btBroadphaseInterface* mOverlappingPairCache				= nullptr; 
			btSequentialImpulseConstraintSolver* mSolver				= nullptr;

			HashSet< RigidBody* > mRigidBodies;
	}; 
}

#endif
