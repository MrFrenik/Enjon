// File: RigidBodyComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Components/RigidBodyComponent.h"
#include "Entity/EntityManager.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"
#include "Physics/PhysicsSubsystem.h"
#include "ImGui/ImGuiManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h" 

namespace Enjon
{ 
	//========================================================================

	void RigidBodyComponent::ExplicitDestructor( )
	{
		// Remove component from physics subsystem's contact events
		Engine::GetInstance( )->GetSubsystemCatalog( )->Get< PhysicsSubsystem >( )->ConstCast< PhysicsSubsystem >( )->RemoveFromContactEvents( static_cast< ComponentHandle< RigidBodyComponent >* >( GetHandle() ) );

		// Delete all subscriptions
		ClearAllCallbacks( );

		// Free memory for rigid body
		mRigidBody->Destroy( ); 
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
		// Want to get a handle from the Physics Subsystem for the rigid body
		mRigidBody = EngineSubsystem( PhysicsSubsystem )->AllocateRigidBodyHandle( );

		mRigidBody->SetResourceHandle( mRigidBody );

		// Initialize the rigidbody
		mRigidBody->Initialize( ); 

		// Set user pointer to this physics component
		mRigidBody->SetUserPointer( GetHandle( ) );
		//mRigidBody->SetUserPointer( this );

		//Set local scale of collision shape
		Vec3 localScale = mEntity->GetLocalScale( );

		mRigidBody->SetLocalScaling( localScale );

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
			if ( !mRigidBody->GetIsKinematic( ) )
			{
				Transform wt = mRigidBody->GetWorldTransform( );
				mEntity->SetLocalPosition( wt.GetPosition(), false );
				mEntity->SetLocalRotation( wt.GetRotation(), false ); 
			}
		}
	}

	//========================================================================

	void RigidBodyComponent::Initialize( )
	{ 
		// Set world transform of rigidbody
		mRigidBody->SetWorldTransform( mEntity->GetWorldTransform( ) );
	}

	//========================================================================

	void RigidBodyComponent::Shutdown( )
	{
		// Set transform of rigidbody and clear forces
		mRigidBody->SetWorldTransform( mEntity->GetWorldTransform( ) );
	}

	//========================================================================

	void RigidBodyComponent::SetShape( CollisionShapeType collisionType )
	{
		// Set shape on rigid body 
		mRigidBody->SetShape( collisionType );

		// Update transform
		UpdateTransform( mEntity->GetWorldTransform( ) );
	}

	//========================================================================

	void RigidBodyComponent::ClearForces( )
	{
		mRigidBody->ClearForces( );
	}

	//========================================================================

	void RigidBodyComponent::SetMass( const f32& mass )
	{
		mRigidBody->SetMass( mass );
	}

	//========================================================================

	f32 RigidBodyComponent::GetMass( ) const
	{
		return mRigidBody->GetMass();
	}

	//========================================================================

	void RigidBodyComponent::SetRestitution( const f32& restitution )
	{
		mRigidBody->SetRestitution( restitution );
	}

	//========================================================================

	void RigidBodyComponent::SetLinearDamping( const f32& damping )
	{
		mRigidBody->SetLinearDamping( damping );
	}

	//========================================================================

	void RigidBodyComponent::SetAngularDamping( const f32& damping )
	{
		mRigidBody->SetAngularDamping( damping );
	}

	//========================================================================

	void RigidBodyComponent::SetFriction( const f32& friction )
	{
		mRigidBody->SetFriction( friction );
	}

	//========================================================================

	void RigidBodyComponent::SetGravity( const Vec3& gravity )
	{
		mRigidBody->SetGravity( gravity );
	}

	//========================================================================

	void RigidBodyComponent::SetLinearVelocity( const Vec3& velocity )
	{
		mRigidBody->SetLinearVelocity( velocity );
	}

	//========================================================================

	void RigidBodyComponent::SetLinearFactor( const iVec3& factor )
	{
		mRigidBody->SetLinearFactor( factor );
	}

	//========================================================================
	
	void RigidBodyComponent::SetAngularFactor( const iVec3& factor )
	{
		mRigidBody->SetAngularFactor( factor );
	}

	//========================================================================

	void RigidBodyComponent::UpdateTransform( const Transform& transform )
	{
		// Reset state position state 
		mRigidBody->SetWorldTransform( transform );
		mRigidBody->SetAwake( true );
	}

	//========================================================================

	void RigidBodyComponent::ForceAwake( )
	{
		mRigidBody->ForceAwake( );
	}

	//========================================================================

	void RigidBodyComponent::Translate( const Vec3& translation )
	{
		mRigidBody->Translate( translation );
	}

	//========================================================================

	void RigidBodyComponent::ApplyCentralForce( const Vec3& force )
	{
		mRigidBody->ApplyCentralForce( force );
	}

	//========================================================================

	void RigidBodyComponent::ApplyRelativeForce( const Vec3& force, const Vec3& relativePosition )
	{
		mRigidBody->ApplyRelativeForce( force, relativePosition );
	}

	//========================================================================

	void RigidBodyComponent::ApplyCentralImpulse( const Vec3& force )
	{
		mRigidBody->ApplyCentralImpulse( force );
	}

	//========================================================================

	void RigidBodyComponent::ApplyImpulse( const Vec3& force, const Vec3& relativePosition )
	{
		mRigidBody->ApplyImpulse( force, relativePosition );
	}

	//========================================================================

	CollisionShapeType RigidBodyComponent::GetShapeType( ) const
	{
		return mRigidBody->GetShapeType( );
	}

	//========================================================================

	void RigidBodyComponent::SetContinuousCollisionDetectionEnabled( bool enabled )
	{
		mRigidBody->SetContinuousCollisionDetectionEnabled( enabled );
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
		auto contacts = physx->GetContactList( static_cast< ComponentHandle< RigidBodyComponent >* >( GetHandle() ) );

		// Fill out entities to return
		Vector<EntityHandle> entities;

		if ( contacts )
		{
			for ( auto& c : *contacts )
			{
				entities.push_back( c->Get()->GetEntity( ) );
			} 
		}

		return entities; 
	}

	//======================================================================== 

	Result RigidBodyComponent::SerializeData( ByteBuffer* buffer ) const 	
	{
		// Serialize out mBody
		return ObjectArchiver::Serialize( mRigidBody.get(), buffer ); 
	}

	//======================================================================== 

	Result RigidBodyComponent::DeserializeData( ByteBuffer* buffer )
	{
		// Deserialize mBody 
		return ObjectArchiver::Deserialize( buffer, mRigidBody.get_raw_ptr() ); 
	}

	//======================================================================== 

	Result RigidBodyComponent::DeserializeLateInit( )
	{
		// Reinitialize rigidbody
		mRigidBody->Reinitialize( ); 

		// Reset world transform
		if ( mEntity )
		{
			mRigidBody->SetWorldTransform( mEntity->GetWorldTransform( ) );
		}

		return Result::SUCCESS;
	}

	//======================================================================== 

	Result RigidBodyComponent::OnEditorUI( )
	{
		// Inspect rigid body 
		EngineSubsystem( ImGuiManager )->InspectObject( mRigidBody.get_raw_ptr() ); 

		return Result::SUCCESS;
	}

	//======================================================================== 

	bool RigidBodyComponent::IsKinematic( )
	{
		return mRigidBody->GetIsKinematic( );
	}
	
	Transform RigidBodyComponent::GetWorldTransform( )
	{
		return mRigidBody->GetWorldTransform( );
	} 

	//======================================================================== 

	void RigidBodyComponentSystem::Update( )
	{ 
		PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
		if ( !phys->IsPaused( ) )
		{ 
			// Grab all rigid body component data from entity manager
			EntityManager* em = EngineSubsystem( EntityManager );
			ComponentInstanceData< RigidBodyComponent >* data = em->GetIComponentInstanceData< RigidBodyComponent >( ); 
			RigidBodyComponent* rbcd = data->Data( );

			for ( usize i = 0; i < data->GetDataSize(); ++i )
			{
				RigidBodyComponent& rbc = rbcd[ i ]; 
				if ( rbc.IsKinematic( ) )
				{
					Transform wt = rbc.GetWorldTransform( );
					Entity* ent = rbc.GetEntity( );
					ent->SetWorldPosition( wt.GetPosition(), false );
					ent->SetWorldRotation( wt.GetRotation( ), false );
				}
			} 
		} 
	}

	//======================================================================== 

	Result RigidBodyComponentSystem::PostComponentConstruction( const u32& id, IComponentInstanceData* data )
	{
		return Result::SUCCESS;
	} 

	//======================================================================== 

}