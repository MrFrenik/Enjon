// File: PhysicsComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Components/PhysicsComponent.h"
#include "Entity/EntityManager.h"

namespace Enjon
{ 
	//========================================================================

	PhysicsComponent::PhysicsComponent( )
	{ 
	}

	//========================================================================

	PhysicsComponent::~PhysicsComponent( )
	{
		// Remove body from physics world
		if ( mBody )
		{
			Engine::GetInstance( )->GetSubsystem( Object::GetClass< PhysicsSubsystem >( ) )->ConstCast< PhysicsSubsystem >( )->RemoveBody( mBody );
		}

		delete mMotionState;
		delete mBody;
		delete mShape;
		mBody = nullptr;
		mShape = nullptr;
		mMotionState = nullptr;
	}

	//========================================================================

	void PhysicsComponent::PostConstruction( )
	{
		// Construct rigid body info
		btVector3 localInertia( 0.0f, 0.0f, 0.0f );
		if ( mMass != 0.0f )
		{
			mShape->calculateLocalInertia( mMass, localInertia );
		}

		// Transform ( Entity is not set yet, so we default to origin )
		btTransform transform;
		transform.setIdentity( );
		transform.setOrigin( btVector3( 0.0f, 0.0f, 0.0f ) );

		// Motion state
		mMotionState = new btDefaultMotionState( transform );
		btRigidBody::btRigidBodyConstructionInfo rbInfo( mMass, mMotionState, mShape, localInertia );
		mBody = new btRigidBody( rbInfo );
		mBody->setRestitution( mRestitution );
		mBody->setFriction( mFriction );
		mBody->setDamping( mLinearDamping, mAngularDamping );
		mBody->setGravity( btVector3( mGravity.x, mGravity.y, mGravity.z ) );

		// Set user pointer to physics component
		mBody->setUserPointer( this );

		// Get physics subsystem
		PhysicsSubsystem* phys = Engine::GetInstance( )->GetSubsystem( Object::GetClass<PhysicsSubsystem>( ) )->ConstCast< PhysicsSubsystem >( );

		// Add body to physics world and set physics world pointer
		phys->AddBody( this->mBody );
		mWorld = phys->GetWorld( ); 
	}

	void PhysicsComponent::Initialize( )
	{ 
		//Set local scale
		Vec3 localScale = mEntity->GetLocalScale( );
		mBody->getCollisionShape( )->setLocalScaling( btVector3( localScale.x, localScale.y, localScale.z ) );

		// Construct rigid body info
		btVector3 localInertia( 0.0f, 0.0f, 0.0f );
		if ( mMass != 0.0f )
		{
			mShape->calculateLocalInertia( mMass, localInertia );
		}

		// Set transform
		UpdateTransform( mEntity->GetWorldTransform( ) );
	}

	void PhysicsComponent::Update( const f32& dt )
	{ 
		// Set world position of entity from physics
		btTransform trans;
		if ( mBody->getMotionState( ) )
		{
			mBody->getMotionState( )->getWorldTransform( trans );
			btVector3 origin = trans.getOrigin( );
			btQuaternion rot = trans.getRotation( );
			Vec3 pos = Vec3( origin.getX( ), origin.getY( ), origin.getZ( ) );
			Quaternion rotation = Quaternion( rot.x( ), rot.y( ), rot.z( ), -rot.w( ) );
			mEntity->SetLocalPosition( pos, false );
			mEntity->SetLocalRotation( rotation, false );
		}
	}

	//========================================================================

	void PhysicsComponent::ClearForces( )
	{
		mBody->clearForces( );
	}

	//========================================================================

	void PhysicsComponent::SetMass( const f32& mass )
	{ 
		mMass = mass;

		// Construct rigid body info
		btVector3 localInertia( 0.0f, 0.0f, 0.0f );
		if ( mMass != 0.0f )
		{
			mShape->calculateLocalInertia( mMass, localInertia );
		}
		mBody->setMassProps( mMass, localInertia );
	}

	//========================================================================

	void PhysicsComponent::SetRestitution( const f32& restitution )
	{ 
		mRestitution = restitution;
		mBody->setRestitution( restitution );
	}

	//========================================================================

	void PhysicsComponent::SetLinearDamping( const f32& damping )
	{
		mLinearDamping = damping;
		mBody->setDamping( mLinearDamping, mAngularDamping );
	}

	//========================================================================

	void PhysicsComponent::SetAngularDamping( const f32& damping )
	{ 
		mAngularDamping = damping;
		mBody->setDamping( mLinearDamping, mAngularDamping );
	}

	//========================================================================

	void PhysicsComponent::SetFriction( const f32& friction )
	{
		mFriction = friction;
		mBody->setFriction( friction );
	}

	//========================================================================

	void PhysicsComponent::SetGravity( const Vec3& gravity )
	{
		mGravity = gravity;
		mBody->setGravity( btVector3( gravity.x, gravity.y, gravity.z ) );
	}

	//========================================================================

	void PhysicsComponent::UpdateTransform( const Transform& transform )
	{ 
		// Create new transform
		btTransform bTransform;
		bTransform.setIdentity( );
		const Vec3* elp = &transform.Position;
		const Quaternion* elr = &transform.Rotation;
		const Vec3* els = &transform.Scale;
		bTransform.setOrigin( btVector3( elp->x, elp->y, elp->z ) );
		bTransform.setRotation( btQuaternion( elr->x, elr->y, elr->z, elr->w ) );
		mBody->getCollisionShape( )->setLocalScaling( btVector3( els->x, els->y, els->z ) );

		// Reset state position state
		mBody->setWorldTransform( bTransform );
		mBody->getMotionState( )->setWorldTransform( bTransform );

		// Clear forces
		ClearForces( ); 
	}

	//========================================================================

	void PhysicsComponent::ForceAwake( )
	{
		mBody->activate( true );
	}

	//========================================================================

	void PhysicsComponent::ApplyCentralForce( const Vec3& force )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyCentralForce( btVector3( force.x, force.y, force.z ) );
	}

	//========================================================================

	void PhysicsComponent::ApplyRelativeForce( const Vec3& force, const Vec3& relativePosition )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyForce( btVector3( force.x, force.y, force.z ), btVector3( relativePosition.x, relativePosition.y, relativePosition.z ) );
	}

	//========================================================================

	void PhysicsComponent::ApplyCentralImpulse( const Vec3& force )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyCentralImpulse( btVector3( force.x, force.y, force.z ) );
	}

	//========================================================================

	void PhysicsComponent::ApplyImpulse( const Vec3& force, const Vec3& relativePosition )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyImpulse( btVector3( force.x, force.y, force.z ), btVector3( relativePosition.x, relativePosition.y, relativePosition.z ) ); 
	}

	//========================================================================
}
