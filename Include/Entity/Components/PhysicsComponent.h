//// @file PhysicsComponent.h
//// Copyright 2016-2018 John Jackson. All Rights Reserved.
//
//#pragma once
//#ifndef ENJON_PHYSICS_COMPONENT_H
//#define ENJON_PHYSICS_COMPONENT_H
//
//#include "Base/Object.h"
//#include "Entity/Component.h"
//#include "Physics/PhysicsSubsystem.h"
//#include "System/Types.h"
//
//namespace Enjon
//{ 
//	using CollisionCallback = std::function< void( const CollisionReport& ) >;
//
//	ENJON_CLASS( )
//	class PhysicsComponent : public Component
//	{
//		ENJON_CLASS_BODY( )
//		ENJON_COMPONENT( PhysicsComponent )
//
//		public: 
//
//			/**
//			* @brief
//			*/
//			PhysicsComponent( );
//
//			/**
//			* @brief
//			*/
//			~PhysicsComponent();
//
//			/**
//			* @brief
//			*/
//			virtual void Initialize( ) override;
//
//			/**
//			* @brief
//			*/
//			virtual void Update( const f32& dt ) override;
//
//			/**
//			* @brief
//			*/
//			void SetMass( const f32& mass );
//
//			/**
//			* @brief
//			*/
//			void SetRestitution( const f32& restitution );
//
//			/**
//			* @brief
//			*/
//			void SetLinearDamping( const f32& damping );
//
//			/**
//			* @brief
//			*/
//			void SetAngularDamping( const f32& damping );
//
//			/**
//			* @brief
//			*/
//			void SetFriction( const f32& friction );
//
//			/**
//			* @brief
//			*/
//			void SetGravity( const Vec3& gravity );
//
//			/**
//			* @brief
//			*/
//			void ClearForces( );
//
//			/**
//			* @brief
//			*/
//			void ForceAwake( );
//
//			/**
//			* @brief
//			*/
//			void ApplyCentralForce( const Vec3& force );
//
//			/**
//			* @brief
//			*/
//			void ApplyRelativeForce( const Vec3& force, const Vec3& relativePosition );
//
//			/**
//			* @brief
//			*/
//			void ApplyCentralImpulse( const Vec3& force );
//
//			/**
//			* @brief
//			*/
//			void ApplyImpulse( const Vec3& force, const Vec3& relativeImpulse ); 
//
//			/**
//			* @brief
//			*/
//			void OnCollisionEnter( const CollisionReport& collision );
//
//			/**
//			* @brief
//			*/
//			void OnCollisionExit( const CollisionReport& collision );
//
//			/**
//			* @brief
//			*/
//			void AddCollisionEnterCallback( const CollisionCallback& callback );
//
//			/**
//			* @brief
//			*/
//			void AddCollisionExitCallback( const CollisionCallback& callback );
//
//		protected:
//
//			/**
//			* @brief
//			*/
//			void PostConstruction( ); 
//
//			/**
//			* @brief
//			*/
//			void UpdateTransform( const Transform& transform );
//
//		protected:
//
//			ENJON_PROPERTY( )
//			f32 mMass = 1.0f;
//
//			ENJON_PROPERTY( )
//			f32 mRestitution = 0.0f;
//
//			ENJON_PROPERTY( )
//			f32 mFriction = 0.8f;
//
//			ENJON_PROPERTY( )
//			f32 mLinearDamping = 0.05f;
//
//			ENJON_PROPERTY( )
//			f32 mAngularDamping = 0.4f;
//
//			ENJON_PROPERTY( )
//			Vec3 mGravity = Vec3( 0.0f, -10.0f, 10.0f ); 
//
//			Vector < CollisionCallback > mCollisionEnterCallbacks;
//			Vector < CollisionCallback > mCollisionExitCallbacks;
//
//			BulletCollisionShape* mShape = nullptr;
//			//BulletRigidBody* mBody = nullptr;
//			BulletRigidBodyMotionState* mMotionState = nullptr;
//			BulletDynamicPhysicsWorld* mWorld = nullptr; 
//	};
//}
//
//#endif
