// @file RigidBodyComponent.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_RIGIDBODY_COMPONENT_H
#define ENJON_RIGIDBODY_COMPONENT_H

#include "Base/Object.h"
#include "Entity/Component.h"
#include "Physics/PhysicsSubsystem.h"
#include "Physics/RigidBody.h"
#include "System/Types.h"

namespace Enjon
{ 
	using CollisionCallback = std::function< void( const CollisionReport& ) >;

	ENJON_CLASS( Construct )
	class RigidBodyComponent : public Component
	{
		ENJON_CLASS_BODY( )
		ENJON_COMPONENT( RigidBodyComponent )

		public: 

			/**
			* @brief
			*/
			RigidBodyComponent( );

			/**
			* @brief
			*/
			~RigidBodyComponent();

			/**
			* @brief
			*/
			virtual void Initialize( ) override;

			/**
			* @brief
			*/
			virtual void Update( const f32& dt ) override;

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
			void SetLinearVelocity( const Vec3& velocity );

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
			void SetShape( CollisionShapeType collisionType );

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
			void OnCollisionEnter( const CollisionReport& collision );

			/**
			* @brief
			*/
			void OnCollisionExit( const CollisionReport& collision );

			/**
			* @brief
			*/
			void AddCollisionEnterCallback( const CollisionCallback& callback );

			/**
			* @brief
			*/
			void AddCollisionExitCallback( const CollisionCallback& callback );

		protected:

			/**
			* @brief
			*/
			void PostConstruction( ); 

			/**
			* @brief
			*/
			virtual void UpdateTransform( const Transform& transform ) override;

		protected: 
			ENJON_PROPERTY( )
			RigidBody mBody;

		protected: 
			Vector < CollisionCallback > mCollisionEnterCallbacks;
			Vector < CollisionCallback > mCollisionExitCallbacks;
	};
}

#endif
