// @file RigidBodyComponent.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_RIGIDBODY_COMPONENT_H
#define ENJON_RIGIDBODY_COMPONENT_H

#include "Base/Object.h"
#include "Entity/Component.h"
#include "Physics/PhysicsSubsystem.h"
#include "Physics/RigidBody.h"
#include "Physics/CollisionReport.h"
#include "System/Types.h"

namespace Enjon
{ 
	using CollisionCallback = std::function< void( Component*, const CollisionReport& ) >; 
 
	class CollisionCallbackSubscriptionBase
	{ 
		public:
			CollisionCallbackSubscriptionBase( )
			{ 
			}

			~CollisionCallbackSubscriptionBase( )
			{
			} 

			virtual void Invoke( const CollisionReport& report ) = 0;
	 };

	template <typename T>
	class CollisionCallbackSubscription : public CollisionCallbackSubscriptionBase
	{
		public:
			CollisionCallbackSubscription( T* comp, const std::function< void(T*, const CollisionReport& )>& callback )
				: mComponent( comp ), mCallback( callback )
			{ 
				static_assert( std::is_base_of<Component, T>::value, "CollisionCallbackSubscription::Constructor() - T must inherit from Component." ); 
			}

			~CollisionCallbackSubscription( )
			{
			} 

			void Invoke( const CollisionReport& report )
			{
				mCallback( mComponent, report );
			}

			T* mComponent = nullptr;
			std::function< void( T*, const CollisionReport& ) > mCallback; 
	};


	ENJON_CLASS( Construct )
	class RigidBodyComponent : public Component
	{
		ENJON_COMPONENT( RigidBodyComponent )

		public: 

			/**
			* @brief
			*/
			void ExplicitDestructor( ) override;

			/**
			* @brief
			*/
			virtual void PostConstruction( ) override; 

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
			virtual void Shutdown( ) override;

			/**
			* @brief
			*/
			void SetMass( const f32& mass );

			/**
			* @brief
			*/
			f32 GetMass( ) const;

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
			void SetLinearFactor( const iVec3& factor );
			
			/**
			* @brief
			*/
			void SetAngularFactor( const iVec3& factor );

			/**
			* @brief
			*/
			void Translate( const Vec3& translation );

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
			CollisionShapeType GetShapeType( ) const;

			/**
			* @brief
			*/
			void SetContinuousCollisionDetectionEnabled( bool enabled ); 

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
			void OnCollisionOverlap( const CollisionReport& collision );

			/**
			* @brief
			*/
			Vector< EntityHandle > GetOverlappingEntities( );

			/**
			* @brief
			*/
			template <typename T>
			void AddCollisionEnterCallback( T* obj, const std::function<void(T*, const CollisionReport& ) >& func )
			{
				mCollisionEnterCallbacks.push_back( new CollisionCallbackSubscription<T>( obj, func ) );
			}

			/**
			* @brief
			*/
			template <typename T>
			void AddCollisionExitCallback( T* obj, const std::function<void(T*, const CollisionReport& ) >& func )
			{
				mCollisionExitCallbacks.push_back( new CollisionCallbackSubscription<T>( obj, func ) );
			}

			template <typename T>
			void AddCollisionOverlapCallback( T* obj, const std::function<void( T*, const CollisionReport& ) >& func )
			{
				mCollisionOverlapCallbacks.push_back( new CollisionCallbackSubscription< T >( obj, func ) );
			}

		protected:

			/**
			* @brief
			*/
			virtual void UpdateTransform( const Transform& transform ) override;
 
			/**
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/**
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

			/**
			* @brief
			*/
			virtual Result DeserializeLateInit( ) override;

			/**
			* @brief
			*/
			void ClearAllCallbacks( );

		protected: 
			ENJON_PROPERTY( )
			RigidBody mBody;

		protected: 
			Vector < CollisionCallbackSubscriptionBase* > mCollisionEnterCallbacks;
			Vector < CollisionCallbackSubscriptionBase* > mCollisionExitCallbacks;
			Vector < CollisionCallbackSubscriptionBase* > mCollisionOverlapCallbacks;
	};
}

#endif
