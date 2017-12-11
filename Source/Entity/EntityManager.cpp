#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Entity/Components/GraphicsComponent.h"
#include "Entity/Components/PointLightComponent.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm> 

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ImGui/ImGuizmo.h"

#include <fmt/printf.h> 
#include <stdio.h>

namespace Enjon 
{ 
	//================================================================================================

	EntityHandle::EntityHandle( )
	{ 
	}
 
	//================================================================================================
		
	EntityHandle::EntityHandle( const Entity* entity )
		: mEntity( entity )
	{
		mID = entity->mID;
	}
 
	//================================================================================================

	EntityHandle::~EntityHandle( )
	{ 
	}
 
	//================================================================================================

	u32 EntityHandle::GetID( ) const
	{
		return mID;
	}

	//================================================================================================

	Enjon::Entity* EntityHandle::Get( ) const
	{ 
		return const_cast< Entity* > ( mEntity );
	}
		
	//================================================================================================

	bool operator==( EntityHandle left, const EntityHandle& other )
	{
		// Compare raw entity pointers and ids for match
		return ( left.Get( ) == other.Get( ) ) && ( left.mID == other.mID );
	} 

	//================================================================================================

	Entity::Entity()
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mComponentMask(Enjon::ComponentBitset(0)),
	  mManager(nullptr),
	  mWorldTransformDirty(true)
	{
	}

	//================================================================================================

	Entity::Entity(EntityManager* manager)
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mComponentMask(Enjon::ComponentBitset(0)),
	  mManager(manager),
	  mWorldTransformDirty(true)
	{
	}

	// TODO(John): Take care of removing all attached components here
	//---------------------------------------------------------------
	Entity::~Entity()
	{
		mManager->Destroy( GetHandle( ) );		
	}

	//---------------------------------------------------------------
			
	EntityHandle Entity::GetHandle( )
	{
		EntityHandle handle( this );
		return handle;
	}

	//---------------------------------------------------------------
	void Entity::Reset()
	{
		assert(mManager != nullptr);

		RemoveParent( );

		// Remove all children and add to parent hierarchy list
		for (auto& c : mChildren)
		{
			Enjon::Entity* e = c.Get( );
			if ( e )
			{
				e ->RemoveParent( ); 
			}
		}

		mID = MAX_ENTITIES;
		mState = EntityState::INACTIVE;
		mWorldTransformDirty = true;
		mComponentMask = Enjon::ComponentBitset(0);
		mManager = nullptr;
		mComponents.clear();
		mChildren.clear();
	}

	//====================================================================================================

	void Entity::Update( const f32& dt )
	{
		// Update all core components
		for ( auto& c : mComponents )
		{
			c->Update( dt );
		}
	}

	//====================================================================================================

	Component* Entity::Attach( const MetaClass* compCls )
	{
		return mManager->Attach( compCls, GetHandle() );
	}

	//====================================================================================================

	bool Entity::HasComponent( const MetaClass* compCls )
	{ 
		return ( ( mComponentMask & Enjon::GetComponentBitMask( compCls->GetTypeId( ) ) ) != 0 );
	}

	//---------------------------------------------------------------
	void Entity::SetID(u32 id)
	{
		mID = id;
	}

	//---------------------------------------------------------------
	void Entity::SetLocalTransform(Transform& transform)
	{
		mLocalTransform = transform;
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	Transform Entity::GetLocalTransform()
	{
		return mLocalTransform;
	}

	//---------------------------------------------------------------
	Transform Entity::GetWorldTransform()
	{
		// Calculate world transform
		CalculateWorldTransform();

		// Return world transform
		return mWorldTransform;
	}

	//===========================================================================

	void Entity::CalculateLocalTransform( ) 
	{
		// RelScale = WorldScale / ParentScale 
		// RelRot	= Inverse(ParentRot) * WorldRot
		// Trans	= [Inverse(ParentRot) * (WorldPos - ParentPosition)] / ParentScale;

		if ( HasParent( ) )
		{
			Enjon::Entity* parent = mParent.Get( );

			Transform parentTransform = parent->GetWorldTransform( );
			Enjon::Quaternion parentInverse = parentTransform.Rotation.Inverse( ).Normalize(); 

			Vec3 relativeScale = mWorldTransform.Scale / parentTransform.Scale;
			Quaternion relativeRot = ( parentInverse * mWorldTransform.Rotation ).Normalize();
			Vec3 relativePos = ( parentInverse * ( mWorldTransform.Position - parentTransform.Position ) ) / parentTransform.Scale;

			mLocalTransform = Transform( relativePos, relativeRot, relativeScale );
		}
	}

	//===========================================================================

	void Entity::CalculateWorldTransform()
	{ 
		// WorldScale = ParentScale * LocalScale
		// WorldRot = LocalRot * ParentRot
		// WorldPos = ParentPos + [ Inverse(ParentRot) * ( ParentScale * LocalPos ) ]

		if ( !HasParent( ) )
		{
			mWorldTransform = mLocalTransform;
			return;
		}

		// Get parent transform recursively
		Enjon::Entity* p = mParent.Get( );
		Transform parent = p->GetWorldTransform( );

		Enjon::Vec3 worldScale = parent.Scale * mLocalTransform.Scale;
		Enjon::Quaternion worldRot = ( mLocalTransform.Rotation * parent.Rotation ).Normalize( );
		Enjon::Vec3 worldPos = parent.Position + ( parent.Rotation.Inverse().Normalize() * ( parent.Scale * mLocalTransform.Position ) );

		mWorldTransform = Transform( worldPos, worldRot, worldScale );

		mWorldTransformDirty = false;
	}

	//===========================================================================

	void Entity::SetPosition(Vec3& position)
	{
		mLocalTransform.SetPosition(position);
		mWorldTransformDirty = true;
	}

	//===========================================================================

	void Entity::SetScale(f32 scale)
	{
		SetScale(v3(scale));
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	void Entity::SetScale(Vec3& scale)
	{
		mLocalTransform.SetScale(scale);
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	void Entity::SetRotation(Quaternion& rotation)
	{
		mLocalTransform.SetRotation(rotation);
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetLocalPosition()
	{
		return mLocalTransform.GetPosition();
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetLocalScale()
	{
		return mLocalTransform.GetScale();
	}

	//---------------------------------------------------------------
	Quaternion Entity::GetLocalRotation()
	{
		return mLocalTransform.GetRotation();
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetWorldPosition()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetPosition();
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetWorldScale()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetScale();
	}

	//---------------------------------------------------------------
	Quaternion Entity::GetWorldRotation()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetRotation();
	}

	//-----------------------------------------
	void Entity::AddChild(const EntityHandle& child)
	{
		Enjon::Entity* ent = child.Get( );

		if ( ent == nullptr )
		{
			return;
		}

		// Set parent to this
		ent->SetParent( GetHandle( ) );

		// Make sure child doesn't exist in vector before pushing back
		auto query = std::find(mChildren.begin(), mChildren.end(), child);
		if (query == mChildren.end())
		{
			// Add child to children list
			mChildren.push_back(child);

			// Calculate its world transform with respect to parent
			ent->CalculateWorldTransform();
		}
		else
		{
			// Log a warning message here
		} 
	}

	//-----------------------------------------
	void Entity::DetachChild( const EntityHandle& child )
	{
		// Make sure child exists
		assert( mManager != nullptr );

		// Find and erase
		mChildren.erase( std::remove( mChildren.begin(), mChildren.end(), child ), mChildren.end() ); 

		// Recalculate world transform of child
		child.Get( )->CalculateWorldTransform();

		// Set parent to nullptr
		child.Get( )->mParent = nullptr;

	} 

	//-----------------------------------------
	void Entity::SetParent( const EntityHandle& parent )
	{
		// Make sure this child doesn't have a parent
		assert(mManager != nullptr); 
		
		// Calculate world transform ( No parent yet, so set world to local )
		CalculateWorldTransform( ); 
		
		// Set parent to this
		mParent = parent.Get( ); 
		
		// Calculate local transform relative to parent
		CalculateLocalTransform( ); 
	}

	//-----------------------------------------
	void Entity::RemoveParent( )
	{ 
		// No need to remove if nullptr
		if ( mParent.Get( ) == nullptr )
		{
			return;
		}

		// Remove child from parent
		mParent.Get( )->DetachChild( GetHandle( ) ); 

		// Set parent to nullptr
		mParent = nullptr; 
	}

	//---------------------------------------------------------------
	b8 Entity::HasChildren()
	{
		return (mChildren.size() > 0);
	}

	//---------------------------------------------------------------
	b8 Entity::HasParent()
	{
		return (mParent.Get( ) != nullptr);
	}

	//---------------------------------------------------------------
	b8 Entity::IsValid()
	{
		return (mState != EntityState::INACTIVE);
	}

	//---------------------------------------------------------------
	void Entity::SetAllChildWorldTransformsDirty()
	{
		for (auto& c : mChildren)
		{
			Enjon::Entity* ent = c.Get( );
			if ( ent )
			{
				// Set dirty to true
				ent->mWorldTransformDirty = true;

				// Iterate through child's children to set their state dirty as well
				ent->SetAllChildWorldTransformsDirty(); 
			}
		}
	}

	void Entity::PropagateTransform(f32 dt)
	{
		// Calculate world transform
		mWorldTransform = mLocalTransform;
		if ( HasParent( ) )
		{
			mWorldTransform *= mParent.Get( )->mWorldTransform;
		}

		// Iterate through children and propagate down
		for (auto& c : mChildren)
		{
			Enjon::Entity* ent = c.Get( );
			if ( ent )
			{
				ent->PropagateTransform( dt );
			}
		}

		UpdateComponentTransforms( dt );
	}

	//---------------------------------------------------------------
	void Entity::UpdateAllChildTransforms()
	{
		// Maybe this should just be to set their flags to dirty?
		for ( auto& c : mChildren )
		{
			Enjon::Entity* ent = c.Get( );
			if ( ent )
			{
				ent->mWorldTransformDirty = true;
				ent->CalculateWorldTransform( );
			}
		}
	}

	//---------------------------------------------------------------
	void Entity::UpdateComponentTransforms(f32 dt)
	{
		if ( mWorldTransformDirty )
		{
			CalculateWorldTransform( );
		}

		for (auto& c : mComponents)
		{
			c->Update(dt);
		}
	}

	//---------------------------------------------------------------
	EntityManager::EntityManager()
	{
		for (auto i = 0; i < mComponents.size(); i++)
		{
			mComponents.at(i) = nullptr;
		}

		mNextAvailableID = 0;
		mEntities = new std::array<Entity, MAX_ENTITIES>;
	}

	//---------------------------------------------------------------
	EntityManager::~EntityManager()
	{
		// Detach all components from entities
		for (u32 i = 0; i < MAX_ENTITIES; ++i)
		{
			Destroy( mEntities->at( i ).GetHandle( ) );	
		}

		delete[] mEntities;

		// Deallocate all components
		for (u32 i = 0; i < mComponents.size(); ++i)
		{
			delete mComponents.at(i);
			mComponents.at(i) = nullptr;
		}
	}

	//---------------------------------------------------------------
	u32 EntityManager::FindNextAvailableID()
	{
		// Iterate from current available id to MAX_ENTITIES
		for (u32 i = mNextAvailableID; i < MAX_ENTITIES; ++i)
		{
			if (mEntities->at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Iterate from 0 to mNextAvailableID
		for (u32 i = 0; i < mNextAvailableID; ++i)
		{
			if (mEntities->at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Other wise return MAX_ENTITIES, since there are no entity slots left
		return MAX_ENTITIES;
	}

	//---------------------------------------------------------------
	Enjon::EntityHandle EntityManager::Allocate( )
	{
		// Grab next available id and assert that it's valid
		u32 id = FindNextAvailableID();

		// Make sure if valid id ( not out of room )
		assert(id < MAX_ENTITIES);

		// Handle to return
		Enjon::EntityHandle handle;

		// Find entity in array and set values
		Entity* entity = &mEntities->at(id);
		handle.mEntity = entity;
		handle.mID = id;
		entity->mID = id;				
		entity->mHandle = handle;
		entity->mState = EntityState::ACTIVE; 
		entity->mManager = this;

		// Push back live entity into active entity vector
		mActiveEntities.push_back(entity); 

		// Return entity handle
		return handle;
	}

	//---------------------------------------------------------------
	void EntityManager::Destroy( const EntityHandle& entity )
	{ 
		// Push for deferred removal from active entities
		mMarkedForDestruction.push_back(entity);
	}

	//--------------------------------------------------------------
	void EntityManager::Cleanup()
	{
		// Move through dirty list and remove from active entities
		for (auto& e : mMarkedForDestruction)
		{
			Entity* ent = e.Get( );
			if ( ent )
			{
				// Destroy all components
				for (auto& c : ent->mComponents)
				{
					c->Destroy();
				}

				// Reset entity
				ent->Reset();

				// Remove from active entities
				mActiveEntities.erase(std::remove(mActiveEntities.begin(), mActiveEntities.end(), ent), mActiveEntities.end()); 
			}

		}

		mMarkedForDestruction.clear();
	}

	//==================================================================================================

	Result EntityManager::Initialize( )
	{ 
		// Register engine components here
		RegisterComponent< GraphicsComponent >( );
		RegisterComponent< PointLightComponent >( );

		return Result::SUCCESS;
	}

	//==================================================================================================

	void EntityManager::Update( const f32 dt )
	{
		// Clean any entities that were marked for destruction
		Cleanup( );

		// Update all components on entities
		for ( auto& e : mActiveEntities )
		{
			e->Update( dt );
		}
	}

	//==================================================================================================

	void EntityManager::LateUpdate( f32 dt )
	{ 
		// Clean any entities that were marked for destruction
		UpdateAllActiveTransforms(dt);
	}

	//==================================================================================================

	Result EntityManager::Shutdown( )
	{ 
		return Result::SUCCESS;
	}

	//================================================================================================== 

	void EntityManager::UpdateAllActiveTransforms(f32 dt)
	{ 
		// Does nothing for now
	}

	//========================================================================================================================

	Component* EntityManager::Attach( const MetaClass* compCls, const Enjon::EntityHandle& handle )
	{
		// Get type id from component class
		u32 compIdx = compCls->GetTypeId( );

		Enjon::Entity* entity = handle.Get( );

		// Assert entity is valid
		assert(entity != nullptr);
		// Check to make sure isn't already attached to this entity
		assert( !entity->HasComponent( compCls ) ); 
		// Entity id
		u32 eid = entity->GetID(); 

		assert(mComponents.at(compIdx) != nullptr); 

		ComponentWrapperBase* base = mComponents[ compIdx ];

		// Create new component and place into map
		Component* component = (Component*)compCls->Construct( );
		component->SetEntity(entity);
		component->SetID(compIdx);
		component->SetBase( base );
		component->mEntityID = entity->mID;
		Component* cmpPtr = base->AddComponent( eid, component ); 
		delete component;

		// Set bitmask field for component
		entity->mComponentMask |= Enjon::GetComponentBitMask( compIdx );

		// Get component ptr and push back into entity components
		entity->mComponents.push_back(cmpPtr);

		return cmpPtr;

		// Otherwise the entity already has the component
		assert(false);

		// Return null to remove warnings from compiler
		return nullptr;

	}
}


















