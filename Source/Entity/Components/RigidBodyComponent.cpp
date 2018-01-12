// File: RigidBodyComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Components/RigidBodyComponent.h"
#include "Entity/EntityManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//========================================================================

	RigidBodyComponent::RigidBodyComponent( )
	{
		// Set up the bullet rigid body
		mBody.Initialize( );
	}

	//========================================================================

	RigidBodyComponent::~RigidBodyComponent( )
	{
		// Remove component from physics subsystem's contact events
		Engine::GetInstance( )->GetSubsystemCatalog( )->Get< PhysicsSubsystem >( )->ConstCast< PhysicsSubsystem >( )->RemoveFromContactEvents( this );

		// Delete all messages
		for ( auto& m : mCollisionEnterCallbacks )
		{
			delete m;
			m = nullptr;
		}

		for ( auto& m : mCollisionExitCallbacks )
		{
			delete m;
			m = nullptr;
		}

		mCollisionExitCallbacks.clear( );
		mCollisionExitCallbacks.clear( );
	}

	//========================================================================
 
	void RigidBodyComponent::Initialize( )
	{
		// Set user pointer to this physics component
		mBody.SetUserPointer( this );

		//Set local scale of collision shape
		Vec3 localScale = mEntity->GetLocalScale( );

		mBody.SetLocalScaling( localScale );

		// Set transform ( this happend RIGHT AFTER setting the local scaling )
		UpdateTransform( mEntity->GetWorldTransform( ) );
	}

	void RigidBodyComponent::Update( const f32& dt )
	{
		Transform wt = mBody.GetWorldTransform( );
		mEntity->SetLocalPosition( wt.Position, false );
		mEntity->SetLocalRotation( wt.Rotation, false );
	}

	//========================================================================

	void RigidBodyComponent::SetShape( CollisionShapeType collisionType )
	{
		// Set shape on rigid body 
		mBody.SetShape( collisionType );

		// Update transform
		UpdateTransform( mEntity->GetWorldTransform( ) );
	}

	//========================================================================

	void RigidBodyComponent::ClearForces( )
	{
		mBody.ClearForces( );
	}

	//========================================================================

	void RigidBodyComponent::SetMass( const f32& mass )
	{
		mBody.SetMass( mass );
	}

	//========================================================================

	void RigidBodyComponent::SetRestitution( const f32& restitution )
	{
		mBody.SetRestitution( restitution );
	}

	//========================================================================

	void RigidBodyComponent::SetLinearDamping( const f32& damping )
	{
		mBody.SetLinearDamping( damping );
	}

	//========================================================================

	void RigidBodyComponent::SetAngularDamping( const f32& damping )
	{
		mBody.SetAngularDamping( damping );
	}

	//========================================================================

	void RigidBodyComponent::SetFriction( const f32& friction )
	{
		mBody.SetFriction( friction );
	}

	//========================================================================

	void RigidBodyComponent::SetGravity( const Vec3& gravity )
	{
		mBody.SetGravity( gravity );
	}

	//========================================================================

	void RigidBodyComponent::SetLinearVelocity( const Vec3& velocity )
	{
		mBody.SetLinearVelocity( velocity );
	}

	//========================================================================

	void RigidBodyComponent::UpdateTransform( const Transform& transform )
	{
		// Reset state position state 
		mBody.SetWorldTransform( transform );
	}

	//========================================================================

	void RigidBodyComponent::ForceAwake( )
	{
		mBody.ForceAwake( );
	}

	//========================================================================

	void RigidBodyComponent::ApplyCentralForce( const Vec3& force )
	{
		mBody.ApplyCentralForce( force );
	}

	//========================================================================

	void RigidBodyComponent::ApplyRelativeForce( const Vec3& force, const Vec3& relativePosition )
	{
		mBody.ApplyRelativeForce( force, relativePosition );
	}

	//========================================================================

	void RigidBodyComponent::ApplyCentralImpulse( const Vec3& force )
	{
		mBody.ApplyCentralImpulse( force );
	}

	//========================================================================

	void RigidBodyComponent::ApplyImpulse( const Vec3& force, const Vec3& relativePosition )
	{
		mBody.ApplyImpulse( force, relativePosition );
	}

	//========================================================================

	void RigidBodyComponent::SetContinuousCollisionDetectionEnabled( bool enabled )
	{
		mBody.SetContinuousCollisionDetectionEnabled( enabled );
	}

	//========================================================================

	void RigidBodyComponent::OnCollisionEnter( const CollisionReport& collision )
	{
		// Callbacks for enter collision
		for ( auto& c : mCollisionEnterCallbacks )
		{ 
			c->Invoke( collision );
		}
	}

	//========================================================================

	void RigidBodyComponent::OnCollisionExit( const CollisionReport& collision )
	{
		// Callbacks for exit collision
		for ( auto& c : mCollisionExitCallbacks )
		{
			c->Invoke( collision );
		}
	}

	//========================================================================

	//void RigidBodyComponent::AddCollisionEnterCallback( const CollisionCallbackSubscription& callback )
	//{
	//	mCollisionEnterCallbacks.push_back( callback );
	//}

	////========================================================================

	//void RigidBodyComponent::AddCollisionExitCallback( const CollisionCallbackSubscription& callback )
	//{
	//	mCollisionExitCallbacks.push_back( callback );
	//}

	//======================================================================== 

}