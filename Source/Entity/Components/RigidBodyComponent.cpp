// File: RigidBodyComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Components/RigidBodyComponent.h"
#include "Entity/EntityManager.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"
#include "ImGui/ImGuiManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{ 
	//========================================================================

	void RigidBodyComponent::ExplicitDestructor( )
	{
		// Remove component from physics subsystem's contact events
		Engine::GetInstance( )->GetSubsystemCatalog( )->Get< PhysicsSubsystem >( )->ConstCast< PhysicsSubsystem >( )->RemoveFromContactEvents( this );

		// Delete all subscriptions
		ClearAllCallbacks( );
	}

	//========================================================================

	void RigidBodyComponent::ClearAllCallbacks( )
	{
		// Delete all subscriptions
		for ( auto& m : mCollisionEnterCallbacks )
		{
			delete m;
			m = nullptr;
		}

		// Delete all subscriptions
		for ( auto& m : mCollisionExitCallbacks )
		{
			delete m;
			m = nullptr;
		}

		// Delete all subscriptions
		for ( auto& m : mCollisionOverlapCallbacks )
		{
			delete m;
			m = nullptr;
		}

		// Clear subscriptions
		mCollisionExitCallbacks.clear( );
		mCollisionExitCallbacks.clear( ); 
		mCollisionOverlapCallbacks.clear( );
	}

	//========================================================================
 
	void RigidBodyComponent::PostConstruction( )
	{
		// Initialize the rigidbody
		mBody.Initialize( );

		// Set user pointer to this physics component
		mBody.SetUserPointer( this );

		//Set local scale of collision shape
		Vec3 localScale = mEntity->GetLocalScale( );

		mBody.SetLocalScaling( localScale );

		// Set transform ( this happend RIGHT AFTER setting the local scaling )
		UpdateTransform( mEntity->GetWorldTransform( ) );
	}

	void RigidBodyComponent::Update( )
	{
		PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
		if ( !phys->IsPaused( ) )
		{ 
			// Need to make sure to only set this if the physics was ticked this scene
			// Only set position and rotation if not kinematic - could make a derived kinematic body component for this
			if ( !mBody.GetIsKinematic( ) )
			{
				Transform wt = mBody.GetWorldTransform( );
				mEntity->SetLocalPosition( wt.GetPosition(), false );
				mEntity->SetLocalRotation( wt.GetRotation(), false ); 
			}
		}
	}

	//========================================================================

	void RigidBodyComponent::Initialize( )
	{ 
		// Set world transform of rigidbody
		mBody.SetWorldTransform( mEntity->GetWorldTransform( ) );
	}

	//========================================================================

	void RigidBodyComponent::Shutdown( )
	{
		// Set transform of rigidbody and clear forces
		mBody.SetWorldTransform( mEntity->GetWorldTransform( ) );
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

	f32 RigidBodyComponent::GetMass( ) const
	{
		return mBody.GetMass();
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

	void RigidBodyComponent::SetLinearFactor( const iVec3& factor )
	{
		mBody.SetLinearFactor( factor );
	}

	//========================================================================
	
	void RigidBodyComponent::SetAngularFactor( const iVec3& factor )
	{
		mBody.SetAngularFactor( factor );
	}

	//========================================================================

	void RigidBodyComponent::UpdateTransform( const Transform& transform )
	{
		// Reset state position state 
		mBody.SetWorldTransform( transform );
		mBody.SetAwake( true );
	}

	//========================================================================

	void RigidBodyComponent::ForceAwake( )
	{
		mBody.ForceAwake( );
	}

	//========================================================================

	void RigidBodyComponent::Translate( const Vec3& translation )
	{
		mBody.Translate( translation );
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

	CollisionShapeType RigidBodyComponent::GetShapeType( ) const
	{
		return mBody.GetShapeType( );
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

	void RigidBodyComponent::OnCollisionOverlap( const CollisionReport& collision )
	{
		// Callbacks for overlap collision
		for ( auto& c : mCollisionOverlapCallbacks )
		{
			c->Invoke( collision );
		}
	}

	//======================================================================== 

	Vector< EntityHandle > RigidBodyComponent::GetOverlappingEntities( )
	{
		// Get physics
		PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );

		// Get set of rigidbodycomponents from contact lists
		const HashSet<RigidBodyComponent*>* contacts = physx->GetContactList( this );

		// Fill out entities to return
		Vector<EntityHandle> entities;

		if ( contacts )
		{
			for ( auto& c : *contacts )
			{
				entities.push_back( c->GetEntity( ) );
			} 
		}

		return entities; 
	}

	//======================================================================== 

	Result RigidBodyComponent::SerializeData( ByteBuffer* buffer ) const 	
	{
		// Serialize out mBody
		return ObjectArchiver::Serialize( &mBody, buffer ); 
	}

	//======================================================================== 

	Result RigidBodyComponent::DeserializeData( ByteBuffer* buffer )
	{
		// Deserialize mBody 
		return ObjectArchiver::Deserialize( buffer, &mBody ); 
	}

	//======================================================================== 

	Result RigidBodyComponent::DeserializeLateInit( )
	{
		// Reinitialize rigidbody
		mBody.Reinitialize( ); 

		// Reset world transform
		if ( mEntity )
		{
			mBody.SetWorldTransform( mEntity->GetWorldTransform( ) );
		}

		return Result::SUCCESS;
	}

	//======================================================================== 

	Result RigidBodyComponent::OnEditorUI( )
	{
		// Inspect rigid body 
		EngineSubsystem( ImGuiManager )->InspectObject( &mBody ); 

		return Result::SUCCESS;
	}

	//======================================================================== 

	Result RigidBodyComponent::OnViewportDebugDraw()
	{
		return mBody.OnViewportDebugDraw();
	}

	//======================================================================== 

}