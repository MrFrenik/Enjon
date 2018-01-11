// @file RigidBody.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_RIGIDBODY_H
#define ENJON_RIGIDBODY_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Math/Maths.h"
#include "Physics/CollisionShape.h"
#include "Physics/PhysicsDefines.h" 

namespace Enjon
{ 
	ENJON_CLASS( )
	class RigidBody : public Object
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief
			*/
			RigidBody( );

			/**
			* @brief
			*/
			~RigidBody( );

			/**
			* @brief
			*/
			void Initialize( );

			/**
			* @brief
			*/
			void Reinitialize( );

			/**
			* @brief
			*/
			void AddToWorld( );

			/**
			* @brief
			*/
			void RemoveFromWorld( );

			/**
			* @brief
			*/
			void SetShape( CollisionShapeType type );

			/**
			* @brief
			*/
			void SetMass( const f32& mass );

			/**
			* @brief
			*/
			void SetRestitution( const f32& restitution );

			/**
			* @brief
			*/
			void SetLinearDamping( const f32& damping );

			/**
			* @brief
			*/
			void SetAngularDamping( const f32& damping );

			/**
			* @brief
			*/
			void SetFriction( const f32& friction );

			/**
			* @brief
			*/
			void SetGravity( const Vec3& gravity );

			/**
			* @brief
			*/
			void SetLinearVelocity( const Vec3& gravity );

			/**
			* @brief
			*/
			void ClearForces( );

			/**
			* @brief
			*/
			void ForceAwake( );

			/**
			* @brief
			*/
			void ApplyCentralForce( const Vec3& force );

			/**
			* @brief
			*/
			void ApplyRelativeForce( const Vec3& force, const Vec3& relativePosition );

			/**
			* @brief
			*/
			void ApplyCentralImpulse( const Vec3& force );

			/**
			* @brief
			*/
			void ApplyImpulse( const Vec3& force, const Vec3& relativeImpulse ); 

			/**
			* @brief
			*/
			BulletRigidBody* GetRawBody( ) const;

			/**
			* @brief
			*/
			BulletRigidBodyMotionState* RigidBody::GetMotionState( ) const; 

			/**
			* @brief
			*/
			CollisionShape* GetCollisionShape( ) const;

			/**
			* @brief
			*/
			void SetWorldTransform( const Transform& transform );

			/**
			* @brief
			*/
			Transform GetWorldTransform( );

			/**
			* @brief
			*/
			void SetLocalScaling( const Vec3& scale );

			/**
			* @brief
			*/
			void SetUserPointer( void* pointer );

		// Serialized properties
		private:

			ENJON_PROPERTY( )
			f32 mMass = 1.0f;

			ENJON_PROPERTY( )
			f32 mRestitution = 0.0f;

			ENJON_PROPERTY( )
			f32 mFriction = 0.8f;

			ENJON_PROPERTY( )
			f32 mLinearDamping = 0.05f;

			ENJON_PROPERTY( )
			f32 mAngularDamping = 0.4f;

			ENJON_PROPERTY( )
			Vec3 mGravity = Vec3( 0.0f, -10.0f, 10.0f );

			ENJON_PROPERTY( )
			CollisionShape* mShape = nullptr;

		private:
			BulletRigidBody* mBody = nullptr;
			BulletRigidBodyMotionState* mMotionState = nullptr;
			BulletDynamicPhysicsWorld* mWorld = nullptr; 
	};
}

#endif
