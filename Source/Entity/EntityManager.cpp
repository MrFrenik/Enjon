#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Entity/Components/GraphicsComponent.h"
#include "Entity/Components/PointLightComponent.h"
#include "Entity/Components/RigidBodyComponent.h"
#include "SubsystemCatalog.h"
#include "Application.h"
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
		if ( mEntity )
		{
			mID = mEntity->mID;
		}
	}
 
	//================================================================================================

	EntityHandle::~EntityHandle( )
	{ 
	}
 
	//================================================================================================

	EntityHandle::operator bool( )
	{
		return ( Get( ) != nullptr );
	}
 
	//================================================================================================

	EntityHandle EntityHandle::Invalid( )
	{
		return EntityHandle( );
	}
 
	//================================================================================================

	u32 EntityHandle::GetID( ) const
	{
		return mID;
	}

	//================================================================================================

	Enjon::Entity* EntityHandle::Get( ) const
	{ 
		EntityManager* manager = EngineSubsystem( EntityManager );
		return manager->GetRawEntity( mID );
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
	  mManager(nullptr),
	  mWorldTransformDirty(true)
	{
	}

	//================================================================================================

	Entity::Entity(EntityManager* manager)
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mManager(manager),
	  mWorldTransformDirty(true)
	{
	}

	//=================================================================

	Entity::~Entity()
	{
		this->Destroy( );
	}

	//=================================================================
			
	EntityHandle Entity::GetHandle( )
	{
		return EntityHandle( this );
	}

	void Entity::Destroy( )
	{
		if ( mManager )
		{
			mManager->Destroy( GetHandle( ) ); 
		}
	} 

	//=================================================================

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
				// Remove but don't remove from list just yet since we're iterating it
				e ->RemoveParent( true ); 

			}
		} 

		// Reset all fields
		mLocalTransform = Enjon::Transform( );
		mWorldTransform = Enjon::Transform( ); 
		mID = MAX_ENTITIES;
		mState = EntityState::INACTIVE;
		mWorldTransformDirty = true;
		mManager = nullptr;
		mComponents.clear();
		mChildren.clear();
	}

	//====================================================================================================

	void Entity::Update( const f32& dt )
	{
		const Application* app = Engine::GetInstance()->GetApplication( );

		// Update all components
		for ( auto& c : mComponents )
		{
			auto comp = mManager->GetComponent( GetHandle(), c );
			if ( comp )
			{
				if ( comp->GetTickState() == ComponentTickState::TickAlways || app->GetApplicationState( ) == ApplicationState::Running )
				{
					comp->Update( dt ); 
				}
			}
		} 
	}

	//====================================================================================================

	void Entity::RemoveComponent( const MetaClass* cls )
	{
		mManager->RemoveComponent( cls, GetHandle( ) );
	}

	//====================================================================================================

	Component* Entity::AddComponent( const MetaClass* compCls )
	{
		return mManager->AddComponent( compCls, GetHandle() );
	}

	//====================================================================================================

	bool Entity::HasComponent( const MetaClass* compCls )
	{ 
		return ( std::find( mComponents.begin( ), mComponents.end( ), compCls->GetTypeId( ) ) != mComponents.end( ) );
	}

	//---------------------------------------------------------------
	void Entity::SetID(u32 id)
	{
		mID = id;
	}

	//---------------------------------------------------------------

	Vector<Component*> Entity::GetComponents( )
	{
		Vector<Component*> compReturns;
		for ( auto& c : mComponents )
		{
			auto comp = mManager->GetComponent( GetHandle( ), c );
			if ( comp )
			{
				compReturns.push_back( comp );
			}
		}

		return compReturns;
	}

	//---------------------------------------------------------------

	void Entity::SetLocalTransform( const Transform& transform, bool propagateToComponents )
	{
		mLocalTransform = transform;

		if ( propagateToComponents )
		{ 
			CalculateWorldTransform( );
			UpdateComponentTransforms( );
		}

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

	void Entity::UpdateComponentTransforms( )
	{
		for ( auto& c : GetComponents( ) )
		{
			c->UpdateTransform( mWorldTransform );
		}
	}

	//===========================================================================

	void Entity::SetLocalPosition( Vec3& position, bool propagateToComponents )
	{
		mLocalTransform.SetPosition(position); 

		if ( propagateToComponents )
		{
			CalculateWorldTransform( ); 
			UpdateComponentTransforms( ); 
		}

		mWorldTransformDirty = true;
	}

	//===========================================================================

	void Entity::SetLocalScale( f32 scale, bool propagateToComponents )
	{
		SetLocalScale(v3(scale)); 

		if ( propagateToComponents )
		{
			CalculateWorldTransform( ); 
			UpdateComponentTransforms( ); 
		}

		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	void Entity::SetLocalScale( Vec3& scale, bool propagateToComponents )
	{
		mLocalTransform.SetScale(scale);

		if ( propagateToComponents )
		{
			CalculateWorldTransform( ); 
			UpdateComponentTransforms( ); 
		}

		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	void Entity::SetLocalRotation( Quaternion& rotation, bool propagateToComponents )
	{
		mLocalTransform.SetRotation(rotation);

		if ( propagateToComponents )
		{
			CalculateWorldTransform( ); 
			UpdateComponentTransforms( ); 
		}

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
	void Entity::DetachChild( const EntityHandle& child, bool deferRemovalFromList )
	{
		// Make sure child exists
		assert( mManager != nullptr );

		// Find and erase
		if ( !deferRemovalFromList )
		{
			mChildren.erase( std::remove( mChildren.begin(), mChildren.end(), child ), mChildren.end() ); 
		}

		// Recalculate world transform of child
		child.Get( )->CalculateWorldTransform();

		// Set parent to invalid entity handle
		child.Get( )->mParent = EntityHandle( ); 
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
	void Entity::RemoveParent( bool deferRemovalFromList )
	{ 
		// No need to remove if nullptr
		if ( mParent.Get( ) == nullptr )
		{
			return;
		}

		// Remove child from parent
		mParent.Get( )->DetachChild( GetHandle( ), deferRemovalFromList ); 
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

		UpdateComponentTransforms( );
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

	Component* EntityManager::GetComponent( const EntityHandle& entity, const u32& ComponentID )
	{
		if ( mComponents.find( ComponentID ) != mComponents.end( ) )
		{
			return mComponents[ ComponentID ]->GetComponent( entity.GetID( ) );
		}

		return nullptr;
	}
 
	//---------------------------------------------------------------
	EntityManager::EntityManager()
	{
		for (auto i = 0; i < mComponents.size(); i++)
		{
			mComponents.at(i) = nullptr;
		}

		mNextAvailableID = 0;
		//mEntities = new std::array<Entity, MAX_ENTITIES>;
		mEntities.resize( MAX_ENTITIES );
	}

	//---------------------------------------------------------------
	EntityManager::~EntityManager()
	{
		// Detach all components from entities
		for (u32 i = 0; i < MAX_ENTITIES; ++i)
		{
			//Destroy( mEntities->at( i ).GetHandle( ) );	
			Destroy( mEntities.at( i ).GetHandle( ) );	
		} 

		// Force destroy all entities and their components
		ForceCleanup( );

		// Deallocate all components
		for ( auto& c : mComponents )
		{
			delete c.second;
			c.second = nullptr;
		}
	}

	//---------------------------------------------------------------
	u32 EntityManager::FindNextAvailableID()
	{
		// Iterate from current available id to MAX_ENTITIES
		for (u32 i = mNextAvailableID; i < MAX_ENTITIES; ++i)
		{
			if (mEntities.at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Iterate from 0 to mNextAvailableID
		for (u32 i = 0; i < mNextAvailableID; ++i)
		{
			if (mEntities.at(i).mState == EntityState::INACTIVE)
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
		Entity* entity = &mEntities.at(id);
		handle.mEntity = entity;
		handle.mID = id;
		entity->mID = id;				
		entity->mHandle = handle;
		entity->mState = EntityState::ACTIVE; 
		entity->mManager = this;
		//entity->mUUID = UUID::GenerateUUID( );

		// Push back live entity into active entity vector
		mMarkedForAdd.push_back( entity );

		// Return entity handle
		return handle;
	}

	//---------------------------------------------------------------

	Vector<EntityHandle> EntityManager::GetRootLevelEntities( )
	{
		Vector<EntityHandle> entities;
		for ( auto& e : mActiveEntities )
		{
			if ( !e->HasParent( ) )
			{
				entities.push_back( e->GetHandle( ) );
			}
		}

		return entities;
	}

	//---------------------------------------------------------------

	Entity* EntityManager::GetRawEntity( const u32& id )
	{
		if ( id < MAX_ENTITIES && mEntities.at( id ).mState == EntityState::ACTIVE )
		{
			return &mEntities.at( id );
		}

		return nullptr;
	}

	//---------------------------------------------------------------

	void EntityManager::Destroy( const EntityHandle& entity )
	{ 
		// Push for deferred removal from active entities
		mMarkedForDestruction.push_back(entity.GetID()); 
	}

	//==============================================================================

	void EntityManager::DestroyAll( )
	{
		for ( auto& e : mActiveEntities )
		{
			e->Destroy( );
		}

		for ( auto& e : mMarkedForAdd )
		{
			e->Destroy( );
		} 

		mActiveEntities.clear( );
		mMarkedForAdd.clear( );
	}

	void EntityManager::ForceCleanup( )
	{
		Cleanup( );
	}
 
	//==============================================================================

	void EntityManager::Cleanup()
	{
		// Move through dirty list and remove from active entities
		for (auto& e : mMarkedForDestruction)
		{
			if ( e < MAX_ENTITIES )
			{
				Entity* ent = &mEntities.at( e );

				if ( ent && ent->mState == EntityState::ACTIVE )
				{
					// Destroy all components
					for (auto& c : ent->mComponents)
					{
						auto comp = GetComponent( ent->GetHandle( ), c );
						if ( comp )
						{
							comp->Destroy(); 
						}

						delete comp;
						comp = nullptr;
					}

					// Reset entity
					ent->Reset();

					// Remove from active entities
					mActiveEntities.erase(std::remove(mActiveEntities.begin(), mActiveEntities.end(), ent), mActiveEntities.end()); 
				} 
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
		RegisterComponent< RigidBodyComponent >( );

		return Result::SUCCESS;
	}

	//==================================================================================================

	void EntityManager::Update( const f32 dt )
	{ 
		// Clean any entities that were marked for destruction
		Cleanup( );

		// Add all new entities into active entities
		for ( auto& e : mMarkedForAdd )
		{
			mActiveEntities.push_back( e );
		}

		// Clear the marked for add entities
		mMarkedForAdd.clear( );

		// Update all components on entities
		for ( auto& e : mActiveEntities )
		{
			// Only update if state is active 
			// NOTE(): I don't really like this, since it forces an unecessary branch...
			if ( e->mState == EntityState::ACTIVE )
			{
				e->Update( dt ); 
			}
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

	void EntityManager::RegisterComponent( const MetaClass* cls )
	{
		u32 index = cls->GetTypeId( );
		mComponents[index] = new ComponentArray( );
	}

	//========================================================================================================================

	// TODO(): Need to have a destinction here on whether or not the component being 
	// asked to unregistered is an engine-level component or not - Most likely NEVER want to be able to 
	// unregister one of those
	void EntityManager::UnregisterComponent( const MetaClass* cls )
	{
		u32 index = cls->GetTypeId( ); 
	
		// For now will only erase if there are no components attached to any entities
		if ( ComponentBaseExists( index ) && mComponents[index]->IsEmpty() )
		{
			ComponentWrapperBase* base = mComponents[index];
			mComponents.erase( index );
			delete base;
			base = nullptr;
		}
	}

	//========================================================================================================================

	Component* EntityManager::AddComponent( const MetaClass* compCls, const Enjon::EntityHandle& handle )
	{
		// Get type id from component class
		u32 compIdx = compCls->GetTypeId( );

		Enjon::Entity* entity = handle.Get( );

		// Assert entity is valid
		assert(entity != nullptr);

		// Doesn't have component
		if ( !ComponentBaseExists( compIdx ) )
		{
			RegisterComponent( compCls );
		}

		assert(mComponents.at(compIdx) != nullptr); 

		// Entity id
		u32 eid = entity->GetID(); 

		// If component exists, return it
		if ( entity->HasComponent( compCls ) )
		{
			return mComponents.at( compIdx )->GetComponent( entity->mID );
		} 

		ComponentWrapperBase* base = mComponents[ compIdx ];

		// Create new component and place into map
		Component* component = base->AddComponent( compCls, eid );
		if ( component )
		{
			component->SetEntity(entity);
			component->SetID(compIdx);
			component->SetBase( base );
			component->mEntityID = entity->mID;
			component->PostConstruction( ); 

			// Get component ptr and push back into entity components
			entity->mComponents.push_back( compIdx );
		} 

		return component; 
	}

	//=========================================================================================

	void EntityManager::RemoveComponent( const MetaClass* compCls, const EntityHandle& entity )
	{
		auto comp = GetComponent( entity, compCls->GetTypeId() );
		if ( comp )
		{
			comp->Destroy( );
		}
		delete comp;
		comp = nullptr;
	}

	//=========================================================================================

	bool EntityManager::ComponentBaseExists( const u32& compIdx )
	{
		return ( mComponents.find( compIdx ) != mComponents.end( ) );
	}

	//=========================================================================================

	Vector<const MetaClass*> EntityManager::GetComponentMetaClassList( )
	{
		Vector< const MetaClass* > metaClassList;
		for ( auto& c : mComponents )
		{
			const MetaClass* cls = Object::GetClass( c.first );
			if ( cls )
			{
				metaClassList.push_back( cls );
			}
		}

		return metaClassList;
	}

	//=========================================================================================

}


















