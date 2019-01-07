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
		ENJON_CLASS_BODY( RigidBody )

		public: 

			/**
			* @brief
			*/
			RigidBody( const CollisionShapeType& type );

			/**
			* @brief
			*/
			void Destroy( );

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( ) override;

			/**
			* @brief
			*/
			virtual Result OnEditorUI( ) override;

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
			void SetAwake( bool enable );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetShape( CollisionShapeType type );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetMass( const f32& mass );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			f32 GetMass( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetIsTriggerVolume( bool enable );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			u32 GetIsTriggerVolume( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetRestitution( const f32& restitution );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			f32 GetRestitution( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetLinearDamping( const f32& damping );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			f32 GetLinearDamping( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetAngularDamping( const f32& damping );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			f32 GetAngularDamping( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetFriction( const f32& friction );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			f32 GetFriction( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetGravity( const Vec3& gravity );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			Vec3 GetGravity( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetLinearVelocity( const Vec3& velocity );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetAngularVelocity( const Vec3& velocity );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetLinearFactor( const iVec3& factor );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			iVec3 GetLinearFactor( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetAngularFactor( const iVec3& factor );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			iVec3 GetAngularFactor( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetIsKinematic( bool enable );

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			bool GetIsKinematic( ) const;

			/**
			* @brief
			*/
			void Translate( const Vec3& translation );

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
			void RefreshTransform( );

			/**
			* @brief
			*/
			void SetLocalScaling( const Vec3& scale );

			/**
			* @brief
			*/
			void SetContinuousCollisionDetectionEnabled( bool enabled );

			/**
			* @brief
			*/
			bool GetContinuousCollisionDetectionEnabled( ) const;

			/**
			* @brief
			*/
			void SetUserPointer( void* pointer ); 

			/**
			* @brief
			*/
			CollisionShapeType GetShapeType( ) const; 

		// Serialized properties
		private:

			ENJON_PROPERTY( UIMin = 0.0f, UIMax = 100.0f, Delegates[ Accessor = GetMass, Mutator = SetMass ] )
			f32 mMass = 1.0f;

			ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f, Delegates[ Accessor = GetRestitution, Mutator = SetRestitution ] )
			f32 mRestitution = 0.0f;

			ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f, Delegates[ Accessor = GetFriction, Mutator = SetFriction ] )
			f32 mFriction = 0.8f;

			ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f, Delegates[ Accessor = GetLinearDamping, Mutator = SetLinearDamping ] )
			f32 mLinearDamping = 0.05f;

			ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f, Delegates[ Accessor = GetAngularDamping, Mutator = SetAngularDamping ] )
			f32 mAngularDamping = 0.4f;

			ENJON_PROPERTY( Delegates[ Accessor = GetGravity, Mutator = SetGravity ] )
			Vec3 mGravity = Vec3( 0.0f, -10.0f, 0.0f );

			ENJON_PROPERTY( Delegates[ Accessor = GetContinuousCollisionDetectionEnabled, Mutator = SetContinuousCollisionDetectionEnabled ] )
			bool mCCDEnabled = false;

			ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f, Delegates[ Accessor = GetIsTriggerVolume, Mutator = SetIsTriggerVolume ] )
			bool mIsTriggerVolume = false;

			ENJON_PROPERTY( UIMin = 0, UIMax = 1, Delegates[ Accessor = GetLinearFactor, Mutator = SetLinearFactor ] )
			iVec3 mLinearFactor = iVec3( 1 );

			ENJON_PROPERTY( UIMin = 0, UIMax = 1, Delegates[ Accessor = GetAngularFactor, Mutator = SetAngularFactor ] )
			iVec3 mAngularFactor = iVec3( 1 );

			ENJON_PROPERTY( Delegates[ Accessor = GetIsKinematic, Mutator = SetIsKinematic ] )
			bool mIsKinematic = false; 

		public:
			ENJON_PROPERTY( HideInEditor )
			CollisionShape* mShape = nullptr;

		private:
			BulletRigidBody* mBody = nullptr;
			BulletRigidBodyMotionState* mMotionState = nullptr;
			BulletDynamicPhysicsWorld* mWorld = nullptr; 
	};
}

#endif
