// File: PhysicsComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/RigidBody.h"
#include "Physics/PhysicsSubsystem.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/BoxCollisionShape.h"
#include "Physics/SphereCollisionShape.h"
#include "Physics/EmptyCollisionShape.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/ByteBuffer.h"

#include <Bullet/btBulletCollisionCommon.h>

namespace Enjon
{ 
	//========================================================================

	RigidBody::RigidBody( )
	{ 
	}

	//========================================================================

	RigidBody::RigidBody( const CollisionShapeType& type )
	{
		switch ( type )
		{
			case CollisionShapeType::Box:
			{
				mShape = new BoxCollisionShape( );
			} break;
			case CollisionShapeType::Sphere:
			{
				mShape = new SphereCollisionShape( );
			} break;
		}
	}

	//========================================================================

	RigidBody::~RigidBody( )
	{
		// Remove body from physics world
		if ( mBody )
		{
			Engine::GetInstance( )->GetSubsystem( Object::GetClass< PhysicsSubsystem >( ) )->ConstCast< PhysicsSubsystem >( )->RemoveBody( this );
			delete mBody;
		}

		// Delete motion state
		if ( mMotionState )
		{
			delete mMotionState; 
		}

		if ( mShape )
		{
			delete mShape;
		}

		// Set all to null
		mShape = nullptr;
		mBody = nullptr;
		mMotionState = nullptr;
	}

	//========================================================================

	void RigidBody::Initialize( )
	{
		// Set up shape ( Box for now as default )
		mShape = new EmptyCollisionShape( );

		// Calculate local inertia using shape
		BV3 localInertia = mShape->CalculateLocalInertia( mMass );

		// Transform ( Entity is not set yet, so we default to origin )
		BTransform transform;
		transform.setIdentity( );
		transform.setOrigin( BV3( 0.0f, 0.0f, 0.0f ) );

		// Motion state
		mMotionState = new BulletDefaultMotionState( transform );
		BulletRigidBodyConstructionInfo rbInfo( mMass, mMotionState, mShape->GetRawShape(), localInertia );
		mBody = new BulletRigidBody( rbInfo );
		mBody->setRestitution( mRestitution );
		mBody->setFriction( mFriction );
		mBody->setDamping( mLinearDamping, mAngularDamping );
		mBody->setGravity( PhysicsUtils::Vec3ToBV3( mGravity ) ); 
		mBody->setLinearFactor( PhysicsUtils::iVec3ToBV3( mLinearFactor ) );
		mBody->setAngularFactor( PhysicsUtils::iVec3ToBV3( mAngularFactor ) );

		// Add body to physics world and set physics world pointer
		AddToWorld( );
	}

	//========================================================================

	void RigidBody::Reinitialize( )
	{
		// Set shape
		mBody->setCollisionShape( mShape->GetRawShape( ) );

		// Calculate local inertia using shape
		BV3 localInertia = mShape->CalculateLocalInertia( mMass );

		// Transform ( Entity is not set yet, so we default to origin )
		BTransform transform;
		transform.setIdentity( );
		transform.setOrigin( BV3( 0.0f, 0.0f, 0.0f ) );

		// Motion state
		mMotionState->setWorldTransform( transform );
		mBody->setRestitution( mRestitution );
		mBody->setFriction( mFriction );
		mBody->setDamping( mLinearDamping, mAngularDamping );
		mBody->setGravity( PhysicsUtils::Vec3ToBV3( mGravity ) ); 
		mBody->setLinearFactor( PhysicsUtils::iVec3ToBV3( mLinearFactor ) );
		mBody->setAngularFactor( PhysicsUtils::iVec3ToBV3( mAngularFactor ) );

		// Set mass properties of bullet rigid body
		mBody->setMassProps( mMass, localInertia );

		// Set whether or not this body is a trigger volume
		SetIsTriggerVolume( GetIsTriggerVolume( ) );

		// Set whether or not this body is using ccd
		SetContinuousCollisionDetectionEnabled( mCCDEnabled );

		// Add body to physics world and set physics world pointer
		AddToWorld( );
	}

	//========================================================================

	void RigidBody::AddToWorld( )
	{
		// Get physics subsystem
		PhysicsSubsystem* phys = Engine::GetInstance( )->GetSubsystem( Object::GetClass<PhysicsSubsystem>( ) )->ConstCast< PhysicsSubsystem >( );

		// Add body to physics world and set physics world pointer
		phys->AddBody( this );
		mWorld = phys->GetWorld( ); 
	}

	//========================================================================

	void RigidBody::RemoveFromWorld( )
	{
		// Get physics subsystem
		PhysicsSubsystem* phys = Engine::GetInstance( )->GetSubsystem( Object::GetClass<PhysicsSubsystem>( ) )->ConstCast< PhysicsSubsystem >( );

		// Add body to physics world and set physics world pointer
		phys->RemoveBody( this );
		mWorld = nullptr;
	}

	//========================================================================

	void RigidBody::ClearForces( )
	{
		mBody->clearForces( );
		// Calculate local inertia using shape
		BTransform initialTransform;

		BV3 localInertia = mShape->CalculateLocalInertia( mMass );
		mBody->setMassProps( mMass, localInertia );
	}

	//========================================================================

	void RigidBody::SetAwake( bool enable )
	{
		mBody->activate( enable );
	}

	//========================================================================

	BulletRigidBody* RigidBody::GetRawBody( ) const
	{
		return mBody;
	}

	//========================================================================

	CollisionShapeType RigidBody::GetShapeType( ) const
	{
		return mShape->GetCollisionShapeType( );
	}

	void RigidBody::SetShape( CollisionShapeType type )
	{ 
		// Setting the shape will remove the body from the world then reset the shape correctly
		//if ( GetCollisionShape( ) && GetCollisionShape( )->GetCollisionShapeType( ) == type )
		//{
		//	return;
		//}


		// Remove previous shape from world, delete, and set to null
		if ( mShape )
		{
			// Remove from physics world
			RemoveFromWorld( );

			// Delete the shape
			delete mShape;

			// Set to nullptr
			mShape = nullptr; 
		}

		// Create new shape based on type 
		switch ( type )
		{
			default:
			case CollisionShapeType::Empty:
			case CollisionShapeType::Box:
			{
				mShape = new BoxCollisionShape( );
			} break;
			
			case CollisionShapeType::Sphere:
			{
				mShape = new SphereCollisionShape( );
			} break;
		}

		// Need to reinitialize body
		Reinitialize( );
	}

	//========================================================================

	void RigidBody::SetMass( const f32& mass )
	{ 
		// Set mass property
		mMass = mass;

		// Construct rigid body info
		BV3 localInertia = mShape->CalculateLocalInertia( mMass );

		// Set mass properties of bullet rigid body
		mBody->setMassProps( mMass, localInertia );
	}

	//========================================================================

	f32 RigidBody::GetMass( ) const
	{
		return mMass;
	}

	//========================================================================

	void RigidBody::SetIsTriggerVolume( bool enable )
	{
		mIsTriggerVolume = enable;

		if ( mIsTriggerVolume )
		{
			mBody->setCollisionFlags( mBody->getCollisionFlags( ) | btCollisionObject::CF_NO_CONTACT_RESPONSE );
		}
		else
		{
			// Switch off if previously enabled
			if ( mBody->getCollisionFlags( ) & btCollisionObject::CF_NO_CONTACT_RESPONSE )
			{
				mBody->setCollisionFlags( mBody->getCollisionFlags( ) & ~btCollisionObject::CF_NO_CONTACT_RESPONSE );
			}
		}
	}

	//========================================================================

	u32 RigidBody::GetIsTriggerVolume( ) const
	{
		return mIsTriggerVolume;
	}

	//========================================================================

	void RigidBody::SetRestitution( const f32& restitution )
	{ 
		mRestitution = restitution;
		mBody->setRestitution( restitution );
	}

	//========================================================================

	void RigidBody::SetLinearDamping( const f32& damping )
	{
		mLinearDamping = damping;
		mBody->setDamping( mLinearDamping, mAngularDamping );
	}

	//========================================================================

	f32 RigidBody::GetLinearDamping( ) const
	{
		return mLinearDamping;
	}

	//========================================================================

	void RigidBody::SetAngularDamping( const f32& damping )
	{ 
		mAngularDamping = damping;
		mBody->setDamping( mLinearDamping, mAngularDamping );
	}

	//========================================================================

	f32 RigidBody::GetAngularDamping( ) const
	{
		return mAngularDamping;
	}

	//========================================================================

	void RigidBody::SetFriction( const f32& friction )
	{
		mFriction = friction;
		mBody->setFriction( friction );
	}

	//========================================================================

	void RigidBody::SetGravity( const Vec3& gravity )
	{
		mGravity = gravity;
		mBody->setGravity( PhysicsUtils::Vec3ToBV3( gravity ) );
	}

	//========================================================================

	void RigidBody::SetLinearVelocity( const Vec3& velocity )
	{
		mBody->setLinearVelocity( PhysicsUtils::Vec3ToBV3( velocity ) );
	}

	//========================================================================

	void RigidBody::SetAngularVelocity( const Vec3& velocity )
	{
		mBody->setAngularVelocity( PhysicsUtils::Vec3ToBV3( velocity ) );
	}

	//========================================================================

	void RigidBody::SetLinearFactor( const iVec3& factor )
	{ 
		mBody->setLinearFactor( PhysicsUtils::iVec3ToBV3( factor ) ); 
		mLinearFactor = factor;
	}

	//========================================================================

	void RigidBody::SetAngularFactor( const iVec3& factor ) 
	{
		mAngularFactor = factor;
		mBody->setAngularFactor( PhysicsUtils::iVec3ToBV3( factor ) );
	}

	//========================================================================

	BulletRigidBodyMotionState* RigidBody::GetMotionState( ) const 
	{
		return mMotionState;
	} 

	//========================================================================

	void RigidBody::ForceAwake( )
	{
		mBody->activate( true );
	}

	//========================================================================

	void RigidBody::SetContinuousCollisionDetectionEnabled( bool enabled )
	{
		mCCDEnabled = enabled;
		mBody->setCcdMotionThreshold( (f32)enabled );
		if ( enabled )
		{
			mBody->setCcdSweptSphereRadius( 0.2f );
		}
	}

	//========================================================================

	void RigidBody::ApplyCentralForce( const Vec3& force )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyCentralForce( PhysicsUtils::Vec3ToBV3( force ) );
	}

	//========================================================================

	void RigidBody::ApplyRelativeForce( const Vec3& force, const Vec3& relativePosition )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyForce( PhysicsUtils::Vec3ToBV3( force ), PhysicsUtils::Vec3ToBV3( relativePosition ) );
	}

	//========================================================================

	void RigidBody::ApplyCentralImpulse( const Vec3& force )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyCentralImpulse( PhysicsUtils::Vec3ToBV3( force ) );
	}

	//========================================================================

	void RigidBody::ApplyImpulse( const Vec3& force, const Vec3& relativePosition )
	{ 
		if ( !mBody->isActive( ) )
		{
			ForceAwake( );
		}

		mBody->applyImpulse( PhysicsUtils::Vec3ToBV3( force ), PhysicsUtils::Vec3ToBV3( relativePosition ) );
	}

	//========================================================================

	void RigidBody::Translate( const Vec3& translation )
	{ 
		// Get the world transform from motion state
		BTransform wt;
		wt = mBody->getCenterOfMassTransform( );
		
		// Apply translation
		wt.getOrigin( ) += PhysicsUtils::Vec3ToBV3( translation ); 
		mBody->setCenterOfMassTransform( wt );
		mBody->getMotionState( )->setWorldTransform( wt ); 
	}

	//========================================================================

	CollisionShape* RigidBody::GetCollisionShape( ) const 
	{
		return mShape;
	} 

	//========================================================================

	void RigidBody::SetWorldTransform( const Transform& transform )
	{
		// Create new bullet transform
		BTransform bTransform;
		bTransform.setIdentity( );
		const Vec3* elp = &transform.Position;
		const Quaternion* elr = &transform.Rotation;
		const Vec3* els = &transform.Scale;
		bTransform.setOrigin( BV3( elp->x, elp->y, elp->z ) );
		bTransform.setRotation( BQuat( elr->x, elr->y, elr->z, -elr->w ) );

		// Set local scaling of shape
		mShape->SetLocalScaling( transform.Scale ); 

		//SetLinearVelocity( 0.0f );
		//SetAngularVelocity( 0.0f );
		mBody->setWorldTransform( bTransform );
		mBody->getMotionState( )->setWorldTransform( bTransform );

		// Clear forces
		//ClearForces( ); 
	}

	//========================================================================

	Transform RigidBody::GetWorldTransform( )
	{
		// Set world position of entity from physics
		Transform returnTrans;
		BTransform trans;
		if ( mBody->getMotionState( ) )
		{
			// Get bullet transform from bullet motion body
			mBody->getMotionState( )->getWorldTransform( trans );
			BV3 origin = trans.getOrigin( );
			BQuat rot = trans.getRotation( );

			// Fill out transform information 
			returnTrans.Position = Vec3( origin.getX( ), origin.getY( ), origin.getZ( ) );
			returnTrans.Rotation = Quaternion( rot.x( ), rot.y( ), rot.z( ), -rot.w( ) );
			returnTrans.Scale = GetCollisionShape( )->GetLocalScaling( );
		} 

		// Return transform information
		return returnTrans;
	}

	//========================================================================

	void RigidBody::SetLocalScaling( const Vec3& localScale )
	{ 
		// Set local scaling of collision shape
		GetCollisionShape( )->SetLocalScaling( localScale ); 
	}

	//========================================================================

	void RigidBody::SetUserPointer( void* pointer )
	{
		mBody->setUserPointer( pointer );
	}

	//========================================================================

	Result RigidBody::SerializeData( ByteBuffer* buffer ) const
	{
		// Write out mass
		buffer->Write< f32 >( mMass );

		// Write out restitution
		buffer->Write< f32 >( mRestitution ); 

		// Write out friction
		buffer->Write< f32 >( mFriction ); 

		// Write out linear damping
		buffer->Write< f32 >( mLinearDamping );

		// Write out angular damping
		buffer->Write< f32 >( mAngularDamping );

		// Write out gravity
		buffer->Write< f32 >( mGravity.x );;
		buffer->Write< f32 >( mGravity.y );;
		buffer->Write< f32 >( mGravity.z );;

		// Write out ccd enabled
		buffer->Write< u32 >( mCCDEnabled );

		// Write out whether or not is trigger
		buffer->Write< u32 >( mIsTriggerVolume );

		// Write out linear factor
		buffer->Write< s32 >( mLinearFactor.x );
		buffer->Write< s32 >( mLinearFactor.y );
		buffer->Write< s32 >( mLinearFactor.z );

		// Write out angular factor
		buffer->Write< s32 >( mAngularFactor.x );
		buffer->Write< s32 >( mAngularFactor.y );
		buffer->Write< s32 >( mAngularFactor.z );

		// Serialize out collision shape
		ObjectArchiver::Serialize( mShape, buffer );

		return Result::SUCCESS;
	}

	//========================================================================

	Result RigidBody::DeserializeData( ByteBuffer* buffer )
	{
		// Read in mass
		SetMass( buffer->Read< f32 >( ) );

		// Read in restitution
		SetRestitution( buffer->Read< f32 >( ) );

		// Read in friction
		SetFriction( buffer->Read< f32 >( ) );

		// Read in linear damping
		SetLinearDamping( buffer->Read< f32 >( ) );

		// Read in angular damping
		SetAngularDamping( buffer->Read< f32 >( ) );

		// Read in gravity
		Vec3 gravity;
		gravity.x = buffer->Read< f32 >( );
		gravity.y = buffer->Read< f32 >( );
		gravity.z = buffer->Read< f32 >( );
		SetGravity( gravity );

		// Read in ccd enabled
		SetContinuousCollisionDetectionEnabled( buffer->Read< u32 >( ) );

		// Read in is trigger volume
		SetIsTriggerVolume( buffer->Read< u32 >( ) );

		// Read in linear factor
		iVec3 linearFactor;
		linearFactor.x = buffer->Read< s32 >( );
		linearFactor.y = buffer->Read< s32 >( );
		linearFactor.z = buffer->Read< s32 >( );
		SetLinearFactor( linearFactor );

		// Read in angular factor
		iVec3 angularFactor;
		angularFactor.x = buffer->Read< s32 >( );
		angularFactor.y = buffer->Read< s32 >( );
		angularFactor.z = buffer->Read< s32 >( );
		SetAngularFactor( angularFactor );

		// Release memory for shape first
		if ( mShape )
		{
			delete mShape;
			mShape = nullptr;
		} 

		// Deserialize collision shape
		mShape = (CollisionShape*)ObjectArchiver::Deserialize( buffer ); 

		// Reset raw collision shape
		mBody->setCollisionShape( mShape->GetRawShape( ) );

		return Result::SUCCESS;
	}

	//========================================================================
}
