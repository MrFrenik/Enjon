// File: PhysicsComponent.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Physics/RigidBody.h"
#include "Physics/PhysicsSubsystem.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/BoxCollisionShape.h"
#include "Physics/SphereCollisionShape.h"
#include "Physics/EmptyCollisionShape.h"
#include "Physics/CylinderCollisionShape.h"
#include "Physics/ConeCollisionShape.h"
#include "Physics/CapsuleCollisionShape.h"
#include "Graphics/GraphicsSubsystem.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/BaseTypeSerializeMethods.h"
#include "Serialize/ByteBuffer.h"
#include "ImGui/ImGuiManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <Bullet/btBulletCollisionCommon.h>

namespace Enjon
{ 
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

	void RigidBody::Destroy( )
	{
		// Remove body from physics world
		if ( mBody )
		{
			Engine::GetInstance( )->GetSubsystem( Object::GetClass< PhysicsSubsystem >( ) )->ConstCast< PhysicsSubsystem >( )->RemoveBody( GetRawBody() );
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

	void RigidBody::ExplicitDestructor( )
	{
		//// Remove body from physics world
		//if ( mBody )
		//{
		//	Engine::GetInstance( )->GetSubsystem( Object::GetClass< PhysicsSubsystem >( ) )->ConstCast< PhysicsSubsystem >( )->RemoveBody( GetRawBody() );
		//	delete mBody;
		//}

		//// Delete motion state
		//if ( mMotionState )
		//{
		//	delete mMotionState;
		//}

		//if ( mShape )
		//{
		//	delete mShape;
		//}

		//// Set all to null
		//mShape = nullptr;
		//mBody = nullptr;
		//mMotionState = nullptr;
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
		BulletRigidBodyConstructionInfo rbInfo( mMass, mMotionState, mShape->GetRawShape( ), localInertia );
		mBody = new BulletRigidBody( rbInfo );
		mBody->setRestitution( mRestitution );
		mBody->setFriction( mFriction );
		mBody->setDamping( mLinearDamping, mAngularDamping );
		mBody->setGravity( PhysicsUtils::Vec3ToBV3( mGravity ) );
		mBody->setLinearFactor( PhysicsUtils::iVec3ToBV3( mLinearFactor ) );
		mBody->setAngularFactor( PhysicsUtils::iVec3ToBV3( mAngularFactor ) );
		SetIsTriggerVolume( mIsTriggerVolume );
		SetIsKinematic( mIsKinematic );

		// Add body to physics world and set physics world pointer
		AddToWorld( );
	}

	//========================================================================

	void RigidBody::Reinitialize( )
	{
		// Set shape
		mBody->setCollisionShape( mShape->GetRawShape( ) );

		// Reset body of shape
		mShape->SetBody( this );

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
		SetIsTriggerVolume( mIsTriggerVolume );
		SetIsKinematic( mIsKinematic );

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
		phys->AddBody( GetRawBody() );
		mWorld = phys->GetWorld( );
	}

	//========================================================================

	void RigidBody::RemoveFromWorld( )
	{
		// Get physics subsystem
		PhysicsSubsystem* phys = Engine::GetInstance( )->GetSubsystem( Object::GetClass<PhysicsSubsystem>( ) )->ConstCast< PhysicsSubsystem >( );

		// Add body to physics world and set physics world pointer
		phys->RemoveBody( GetRawBody() );
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
			{
				mShape = new EmptyCollisionShape( this );
			} break;
			case CollisionShapeType::Box:
			{
				mShape = new BoxCollisionShape( this );
			} break;

			case CollisionShapeType::Sphere:
			{
				mShape = new SphereCollisionShape( this );
			} break;

			case CollisionShapeType::Cylinder:
			{
				mShape = new CylinderCollisionShape( this );
			} break;

			case CollisionShapeType::Capsule:
			{
				mShape = new CapsuleCollisionShape( this ); 
			} break;

			case CollisionShapeType::Cone:
			{
				mShape = new ConeCollisionShape( this ); 
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

	f32 RigidBody::GetRestitution( ) const
	{
		return mRestitution;
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

	f32 RigidBody::GetFriction( ) const
	{
		return mFriction;
	}

	//========================================================================

	void RigidBody::SetGravity( const Vec3& gravity )
	{
		mGravity = gravity;
		mBody->setGravity( PhysicsUtils::Vec3ToBV3( gravity ) );
	}

	//========================================================================

	Vec3 RigidBody::GetGravity( ) const
	{
		return mGravity;
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

	iVec3 RigidBody::GetLinearFactor( ) const 
	{
		return mLinearFactor;
	} 

	//========================================================================

	iVec3 RigidBody::GetAngularFactor( ) const
	{ 
		return mAngularFactor;
	}

	//========================================================================

	void RigidBody::SetAngularFactor( const iVec3& factor )
	{
		mAngularFactor = factor;
		mBody->setAngularFactor( PhysicsUtils::iVec3ToBV3( factor ) );
	}

	//========================================================================

	void RigidBody::SetIsKinematic( bool enable )
	{
		// Set state
		mIsKinematic = enable;

		// If set to kinematic object
		if ( enable )
		{
			mBody->setCollisionFlags( mBody->getCollisionFlags( ) | btCollisionObject::CF_KINEMATIC_OBJECT );
			mBody->setActivationState( DISABLE_DEACTIVATION );
		}
		else
		{
			// Switch off if previously enabled
			if ( mBody->getCollisionFlags( ) & btCollisionObject::CF_KINEMATIC_OBJECT )
			{
				mBody->setCollisionFlags( mBody->getCollisionFlags( ) & ~btCollisionObject::CF_KINEMATIC_OBJECT );
				mBody->activate( true );
			}
		};
	}

	//========================================================================

	bool RigidBody::GetIsKinematic( ) const
	{
		return mIsKinematic;
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

	bool RigidBody::GetContinuousCollisionDetectionEnabled( ) const
	{
		return mCCDEnabled;
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

	void RigidBody::RefreshTransform( )
	{
		// Totally useless function that needs to be handled better...
		SetWorldTransform( GetWorldTransform( ) );
	}

	//========================================================================

	void RigidBody::SetWorldTransform( const Transform& transform )
	{
		// Create new bullet transform
		BTransform bTransform;
		bTransform.setIdentity( );

		const Vec3* elp = &transform.GetPosition();
		const Quaternion* elr = &transform.GetRotation();
		const Vec3* els = &transform.GetScale();
		Vec3 offset = *elr * mShape->GetOffset( ); 

		bTransform.setOrigin( BV3( elp->x + offset.x, elp->y + offset.y, elp->z + offset.z ) );
		bTransform.setRotation( BQuat( elr->x, elr->y, elr->z, elr->w ) );

		// Set local scaling of shape
		mShape->SetLocalScaling( transform.GetScale() ); 

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
			BQuat bRot = trans.getRotation( ); 
			Quaternion rot( bRot.x( ), bRot.y( ), bRot.z( ), bRot.w( ) );
			Vec3 offset = rot * mShape->GetOffset( );

			// Fill out transform information 
			returnTrans.SetPosition( Vec3( origin.getX( ), origin.getY( ), origin.getZ( ) ) - offset );
			returnTrans.SetRotation( rot );
			returnTrans.SetScale( GetCollisionShape( )->GetLocalScaling( ) );
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

	Result RigidBody::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Dump self
		igm->DebugDumpObject( this ); 

		// Inspect collision shape
		if ( mShape )
		{
			// Draw separator line
			igm->Separator( ); 

			// Cache shape type before inspection
			CollisionShapeType curShapeType = mShape->GetCollisionShapeType( ); 
			
			// Inspect shape
			igm->InspectObject( mShape );
 
			 // Reset shape if necessary
			if ( curShapeType != mShape->GetCollisionShapeType( ) )
			{
				// Capture its previous position
				Transform wt = PhysicsUtils::BTransformToTransform( mBody->getWorldTransform( ) );
				wt.SetScale( mShape->GetLocalScaling( ) );

				SetShape( mShape->GetCollisionShapeType( ) );

				// Reset world transform
				SetWorldTransform( wt );
			} 

			// Get bullet transform from bullet motion body
			BTransform trans;
			BV3 aabbMin;
			BV3 aabbMax;
			mBody->getMotionState( )->getWorldTransform( trans ); 
			mShape->GetRawShape( )->getAabb( trans, aabbMin, aabbMax ); 

			// Debug draw physics object
			BTransform wt;
			mBody->getMotionState( )->getWorldTransform( wt );
			EngineSubsystem( PhysicsSubsystem )->GetWorld( )->debugDrawObject( wt, mShape->GetRawShape( ), BV3( 1.0f, 1.0f, 1.0f ) );

			// Draw debug aabb
			GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 
			gfx->DrawDebugAABB( PhysicsUtils::BV3ToVec3( aabbMin ), PhysicsUtils::BV3ToVec3( aabbMax ), Vec3( 0.0f, 1.0f, 0.0f ) ); 
		} 

		return Result::SUCCESS;
	}
}
