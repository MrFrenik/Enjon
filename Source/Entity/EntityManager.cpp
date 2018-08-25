#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Entity/Components/StaticMeshComponent.h"
#include "Entity/Components/PointLightComponent.h"
#include "Entity/Components/DirectionalLightComponent.h"
#include "Entity/Components/RigidBodyComponent.h"
#include "Entity/Components/CameraComponent.h"
#include "Entity/Components/SkeletalMeshComponent.h"
#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "Serialize/EntityArchiver.h"
#include "Base/World.h"
#include "Application.h"
#include "Engine.h"

#include <array>
#include <vector>
#include <assert.h>
#include <algorithm> 

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <fmt/printf.h> 
#include <stdio.h>

namespace Enjon
{
	//================================================================================================

	EntityHandle::EntityHandle( )
		: mID( MAX_ENTITIES )
	{
	}

	//================================================================================================

	EntityHandle::EntityHandle( const Entity* entity )
	{
		if ( entity )
		{
			mID = entity->mID;
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

	bool EntityHandle::IsValid( ) const
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

	void Entity::ExplicitConstructor( )
	{
		mID = MAX_ENTITIES;
		mState = EntityState::INVALID;
		mWorldTransformDirty = true;
		mIsArchetypeRoot = false;
	}

	//=================================================================

	void Entity::ExplicitDestructor( )
	{
		this->Destroy( );
	}

	//=================================================================

	EntityHandle Entity::GetHandle( )
	{
		return EntityHandle( this );
	}

	//=================================================================

	bool Entity::HasPrototypeEntity( ) const
	{
		return ( mPrototypeEntity.IsValid( ) );
	}

	//=================================================================

	EntityHandle Entity::GetPrototypeEntity( ) const
	{
		return mPrototypeEntity;
	}

	//=================================================================

	const EntityState& Entity::GetState( ) const
	{
		return mState;
	}

	//=================================================================

	void Entity::Destroy( )
	{
		EngineSubsystem( EntityManager )->Destroy( GetHandle( ) );
	}

	//=================================================================

	UUID Entity::GetUUID( ) const
	{
		return mUUID;
	}

	//=================================================================

	void Entity::SetArchetype( const AssetHandle< Archetype >& archType )
	{
		mArchetype = archType;
	}

	//=================================================================

	void Entity::SetPrototypeEntity( const EntityHandle& handle )
	{
		// Set handle and add instance
		if ( handle.Get( ) )
		{
			mPrototypeEntity = handle; 
			mPrototypeEntity.Get( )->AddInstance( this );
		}
	}

	//=================================================================

	void Entity::RemovePrototypeEntity( )
	{
		if ( mPrototypeEntity )
		{
			mPrototypeEntity.Get( )->RemoveInstance( this );
			mPrototypeEntity = EntityHandle::Invalid( );
		}
	}

	//=================================================================

	void Entity::AddInstance( const EntityHandle& handle )
	{
		if ( handle.Get( ) )
		{
			mInstancedEntities.insert( handle.GetID( ) );
		}
	}

	//=================================================================

	void Entity::RemoveInstance( const EntityHandle& handle ) 
	{
		if ( handle.Get( ) )
		{
			mInstancedEntities.erase( handle.GetID( ) );
		}
	}

	//================================================================= 

	void Entity::RemoveFromWorld( )
	{
		// If world exists, then remove this entity from it
		if ( mWorld )
		{
			EngineSubsystem( EntityManager )->RemoveEntityFromWorld( this );
		}
	}

	//=================================================================

	void Entity::MoveToWorld( World* world )
	{
		if ( mWorld )
		{
			RemoveFromWorld( );
			EngineSubsystem( EntityManager )->AddEntityToWorld( this, world );
		}
	}

	//=================================================================

	void Entity::Reset( )
	{
		// Clear all property overrides
		ObjectArchiver::ClearAllPropertyOverrides( this );

		RemoveParent( );

		// Remove all children and add to parent hierarchy list
		for ( auto& c : mChildren )
		{
			Enjon::Entity* e = c.Get( );
			if ( e )
			{
				// Remove but don't remove from list just yet since we're iterating it
				e->RemoveParent( true );

			}
		}

		// Remove from prototype's instances
		if ( mPrototypeEntity )
		{
			mPrototypeEntity.Get( )->mInstancedEntities.erase( mID );
			mPrototypeEntity = EntityHandle::Invalid( );
		}

		EntityManager* em = EngineSubsystem( EntityManager );

		// Remove all instanced entities
		for ( auto& i : mInstancedEntities )
		{
			EntityHandle h = em->GetRawEntity( i );
			if ( h )
			{
				h.Get( )->mPrototypeEntity = EntityHandle::Invalid( );
			}
		} 

		// Reset all fields
		mLocalTransform = Enjon::Transform( );
		mWorldTransform = Enjon::Transform( );
		mInstancedEntities.clear( );
		mID = MAX_ENTITIES;
		mState = EntityState::INVALID;
		mWorldTransformDirty = true;
		mComponents.clear( );
		mChildren.clear( );

		// Remove from world
		RemoveFromWorld( );
	}

	//====================================================================================================

	void Entity::Update( const f32& dt )
	{
		EntityManager* em = EngineSubsystem( EntityManager );

		const Application* app = Engine::GetInstance( )->GetApplication( );

		// Update all components
		for ( auto& c : mComponents )
		{
			auto comp = em->GetComponent( GetHandle( ), c );
			if ( comp )
			{
				if ( comp->GetTickState( ) == ComponentTickState::TickAlways || app->GetApplicationState( ) == ApplicationState::Running )
				{
					comp->Update( );
				}
			}
		}
	}

	//====================================================================================================

	void Entity::RemoveComponent( const MetaClass* cls )
	{
		EngineSubsystem( EntityManager )->RemoveComponent( cls, GetHandle( ) );
	}

	//====================================================================================================

	Component* Entity::GetComponent( const MetaClass* compCls )
	{
		return EngineSubsystem( EntityManager )->GetComponent( compCls, GetHandle() );
	}

	//====================================================================================================

	Component* Entity::AddComponent( const MetaClass* compCls )
	{
		return EngineSubsystem( EntityManager )->AddComponent( compCls, GetHandle( ) );
	}

	//====================================================================================================

	bool Entity::HasComponent( const MetaClass* compCls )
	{
		return ( std::find( mComponents.begin( ), mComponents.end( ), compCls->GetTypeId( ) ) != mComponents.end( ) );
	}

	//---------------------------------------------------------------
	void Entity::SetID( u32 id )
	{
		mID = id;
	}

	//---------------------------------------------------------------

	Vector< EntityHandle > Entity::GetChildren( )
	{
		return mChildren;
	}

	//---------------------------------------------------------------

	Vector< EntityHandle > Entity::GetInstancedEntities( )
	{
		EntityManager* em = EngineSubsystem( EntityManager );
		Vector< EntityHandle > handles;
		for ( auto& id : mInstancedEntities )
		{
			handles.push_back( em->GetRawEntity( id ) );
		}
		return handles;
	}

	//---------------------------------------------------------------

	Vector<Component*> Entity::GetComponents( )
	{
		EntityManager* em = EngineSubsystem( EntityManager );
		Vector<Component*> compReturns;
		for ( auto& c : mComponents )
		{
			auto comp = em->GetComponent( GetHandle( ), c );
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

	AssetHandle< Archetype > Entity::GetArchetype( ) const
	{
		return mArchetype;
	}

	//---------------------------------------------------------------

	Transform Entity::GetLocalTransform( )
	{
		return mLocalTransform;
	}

	//---------------------------------------------------------------
	Transform Entity::GetWorldTransform( )
	{
		// Calculate world transform
		CalculateWorldTransform( );

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
			// Grab parent entity
			Enjon::Entity* parent = mParent.Get( );

			// Set local transform relative to parent transform
			mLocalTransform = mWorldTransform / parent->GetWorldTransform( );
		}
	}

	//===========================================================================

	void Entity::CalculateWorldTransform( )
	{
		// WorldScale = ParentScale * LocalScale
		// WorldRot = LocalRot * ParentRot
		// WorldPos = ParentPos + [ ParentRot * ( ParentScale * LocalPos ) ]

		if ( !HasParent( ) )
		{
			mWorldTransform = mLocalTransform;
			return;
		}

		// Get parent transform recursively
		Enjon::Entity* p = mParent.Get( );

		// Set world transform
		mWorldTransform = mLocalTransform * p->GetWorldTransform( );

		// Set world transform flag to being clean
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
		mLocalTransform.SetPosition( position );

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
		SetLocalScale( v3( scale ) ); 
	}

	//---------------------------------------------------------------
	void Entity::SetLocalScale( Vec3& scale, bool propagateToComponents )
	{
		mLocalTransform.SetScale( scale );

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
		mLocalTransform.SetRotation( rotation );

		if ( propagateToComponents )
		{
			CalculateWorldTransform( );
			UpdateComponentTransforms( );
		} 

		mWorldTransformDirty = true;
	}

	//--------------------------------------------------------------- 

	void Entity::SetLocalRotation( Vec3& eulerAngles, bool propagateToComponents )
	{
		mLocalTransform.SetRotation( eulerAngles );

		if ( propagateToComponents )
		{
			CalculateWorldTransform( );
			UpdateComponentTransforms( );
		}

		mWorldTransformDirty = true;
	}

	//--------------------------------------------------------------- 

	Vec3 Entity::GetLocalPosition( )
	{
		return mLocalTransform.GetPosition( );
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetLocalScale( )
	{
		return mLocalTransform.GetScale( );
	}

	//---------------------------------------------------------------
	Quaternion Entity::GetLocalRotation( )
	{
		return mLocalTransform.GetRotation( );
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetWorldPosition( )
	{
		if ( mWorldTransformDirty ) CalculateWorldTransform( );
		return mWorldTransform.GetPosition( );
	}

	//---------------------------------------------------------------
	Vec3 Entity::GetWorldScale( )
	{
		if ( mWorldTransformDirty ) CalculateWorldTransform( );
		return mWorldTransform.GetScale( );
	}

	//---------------------------------------------------------------
	Quaternion Entity::GetWorldRotation( )
	{
		if ( mWorldTransformDirty ) CalculateWorldTransform( );
		return mWorldTransform.GetRotation( );
	}

	//-----------------------------------------

	bool Entity::ExistsInChildHierarchy( const EntityHandle& child )
	{
		Entity* childEnt = child.Get( );
		if ( !childEnt )
		{
			return false;
		}

		bool exists = false;

		// Cannot parent to self
		exists |= ( childEnt == this );

		for ( auto& c : mChildren )
		{
			if ( childEnt == c.Get( ) )
			{
				exists |= true;
			}

			exists |= c.Get( )->ExistsInChildHierarchy( child );
		}

		return exists;
	}

	//-----------------------------------------

	void Entity::AddChild( const EntityHandle& child )
	{
		Enjon::Entity* ent = child.Get( );

		if ( ent == nullptr )
		{
			return;
		}

		// Already in child hierachy, cannot add
		if ( ExistsInChildHierarchy( child ) )
		{
			return;
		}

		// Set parent to this
		ent->SetParent( GetHandle( ) );

		// Make sure child doesn't exist in vector before pushing back
		auto query = std::find( mChildren.begin( ), mChildren.end( ), child );
		if ( query == mChildren.end( ) )
		{
			// Add child to children list
			mChildren.push_back( child );

			// Calculate its world transform with respect to parent
			ent->CalculateWorldTransform( );
		}
		else
		{
			// Log a warning message here
		}
	}

	//-----------------------------------------
	void Entity::DetachChild( const EntityHandle& child, bool deferRemovalFromList )
	{
		// Find and erase
		if ( !deferRemovalFromList )
		{
			mChildren.erase( std::remove( mChildren.begin( ), mChildren.end( ), child ), mChildren.end( ) );
		}

		// Recalculate world transform of child
		child.Get( )->CalculateWorldTransform( );

		// Reset local transform to new world transform
		child.Get( )->mLocalTransform = child.Get( )->mWorldTransform;

		// Set parent to invalid entity handle
		child.Get( )->mParent = EntityHandle( );
	}

	//-----------------------------------------
	void Entity::SetParent( const EntityHandle& parent )
	{
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

	b8 Entity::HasChild( const EntityHandle& child )
	{
		for ( auto& c : mChildren )
		{
			if ( c.Get( ) == child.Get( ) )
			{
				return true;
			}
		}

		return false;
	}

	//---------------------------------------------------------------

	b8 Entity::HasChildren( )
	{
		return ( mChildren.size( ) > 0 );
	}

	//---------------------------------------------------------------
	b8 Entity::HasParent( )
	{
		return ( mParent.Get( ) != nullptr );
	}

	//---------------------------------------------------------------
	b8 Entity::IsValid( )
	{
		return ( mState != EntityState::INVALID );
	}

	//---------------------------------------------------------------
	void Entity::SetAllChildWorldTransformsDirty( )
	{
		for ( auto& c : mChildren )
		{
			Enjon::Entity* ent = c.Get( );
			if ( ent )
			{
				// Set dirty to true
				ent->mWorldTransformDirty = true;

				// Iterate through child's children to set their state dirty as well
				ent->SetAllChildWorldTransformsDirty( );
			}
		}
	}

	void Entity::PropagateTransform( f32 dt )
	{
		// Calculate world transform
		mWorldTransform = mLocalTransform;
		if ( HasParent( ) )
		{
			mWorldTransform *= mParent.Get( )->mWorldTransform;
		}

		// Iterate through children and propagate down
		for ( auto& c : mChildren )
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
	void Entity::UpdateAllChildTransforms( )
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

	Vec3 Entity::Forward( )
	{
		return GetWorldRotation( ) * Vec3::ZAxis( );
	}

	//---------------------------------------------------------------

	Vec3 Entity::Right( )
	{
		return GetWorldRotation( ) * Vec3::XAxis( );
	}

	//---------------------------------------------------------------

	Vec3 Entity::Up( )
	{
		return GetWorldRotation( ) * Vec3::YAxis( );
	}

	//---------------------------------------------------------------

	void Entity::SetUUID( const UUID& uuid )
	{
		mUUID = uuid; 

		// Set uuid in map
		EngineSubsystem( EntityManager )->mEntityUUIDMap[ mUUID.ToString( ) ] = this;
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

	Component* EntityManager::GetComponent( const MetaClass* compCls, const EntityHandle& entity )
	{
		u32 compId = compCls->GetTypeId( );
		return GetComponent( entity, compId );
	}

	//---------------------------------------------------------------

	u32 EntityManager::FindNextAvailableID( )
	{
		// Iterate from current available id to MAX_ENTITIES
		for ( u32 i = mNextAvailableID; i < MAX_ENTITIES; ++i )
		{
			if ( mEntities.at( i ).mState == EntityState::INVALID )
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Iterate from 0 to mNextAvailableID
		for ( u32 i = 0; i < mNextAvailableID; ++i )
		{
			if ( mEntities.at( i ).mState == EntityState::INVALID )
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Other wise return MAX_ENTITIES, since there are no entity slots left
		return MAX_ENTITIES;
	}

	//---------------------------------------------------------------

	bool EntityManager::WorldExists( const World* world )
	{
		return ( mWorldEntityMap.find( world ) != mWorldEntityMap.end( ) );
	}

	//---------------------------------------------------------------

	void EntityManager::AddWorld( const World* world )
	{
		// Construct new entity vector for this world entry
		mWorldEntityMap[ world ] = HashSet< Entity* >( );
	}

	//---------------------------------------------------------------

	Enjon::EntityHandle EntityManager::Allocate( World* world )
	{
		// If no world passed in, grab default world from engine
		if ( !world )
		{
			world = Engine::GetInstance( )->GetWorld( );
		}

		// TODO(John): Move this to being a one-time call at initialization of a new world that needs an entity context
		if ( !WorldExists( world ) )
		{
			AddWorld( world );
		}

		// Grab next available id and assert that it's valid
		u32 id = FindNextAvailableID( );

		// Make sure if valid id ( not out of room )
		assert( id < MAX_ENTITIES );

		// Handle to return
		Enjon::EntityHandle handle;

		// Find entity in array and set values
		Entity* entity = &mEntities.at( id );
		handle.mID = id;
		entity->mID = id;
		entity->mState = EntityState::ACTIVE;
		entity->mUUID = UUID::GenerateUUID( );
		entity->mWorld = world; 

		// Push back live entity into active entity vector
		mMarkedForAdd.push_back( entity );

		// Add to uuid map
		AddToUUIDMap( entity ); 

		// Return entity handle
		return handle;
	}

	//---------------------------------------------------------------

	void EntityManager::AddToUUIDMap( const EntityHandle& entity )
	{
		Entity* ent = entity.Get( );

		// Can't operate on null entities
		if ( !ent )
		{
			return;
		}

		// Add to map
		mEntityUUIDMap[ ent->GetUUID( ).ToString( ) ] = ent;
	}

	//---------------------------------------------------------------

	void EntityManager::RemoveFromUUIDMap( const EntityHandle& entity )
	{
		Entity* ent = entity.Get( );

		if ( !ent )
		{
			return;
		}

		// Erase from map
		mEntityUUIDMap.erase( ent->GetUUID( ).ToString( ) );
	}

	//---------------------------------------------------------------

	Vector<EntityHandle> EntityManager::GetRootLevelEntities( World* world )
	{
		if ( !world )
		{
			world = Engine::GetInstance( )->GetWorld( );
		}

		Vector<EntityHandle> entities;
		if ( WorldExists( world ) )
		{
			for ( auto& e : mWorldEntityMap[ world ] )
			{
				if ( !e->HasParent( ) )
				{
					entities.push_back( e->GetHandle( ) );
				}
			}
		}

		return entities;
	}

	//---------------------------------------------------------------

	EntityHandle EntityManager::GetEntityByUUID( const UUID& uuid )
	{
		String uuidStr = uuid.ToString( );

		if ( mEntityUUIDMap.find( uuidStr ) != mEntityUUIDMap.end( ) )
		{
			return mEntityUUIDMap[ uuidStr ];
		}

		return EntityHandle::Invalid( );

		//// Serach for matching UUID ( Should have map for this rather )
		//for ( auto& e : mActiveEntities )
		//{
		//	if ( e->GetState() != EntityState::INACTIVE && e->GetUUID( ) == uuid )
		//	{
		//		return EntityHandle( e );
		//	}
		//}

		//// Search for matching UUID in marked for add list
		//for ( auto& e : mMarkedForAdd )
		//{
		//	if ( e->GetState() != EntityState::INACTIVE && e->GetUUID( ) == uuid )
		//	{
		//		return EntityHandle( e );
		//	}
		//}

		//for ( auto& w : mWorldEntityMap )
		//{
		//	for ( auto& e : w.second )
		//	{
		//		if ( e->GetState() != EntityState::INACTIVE && e->GetUUID( ) == uuid )
		//		{
		//			return EntityHandle( e );
		//		}
		//	}
		//}

		// Return invalid entity if not found
		//return EntityHandle::Invalid( );
	}

	//---------------------------------------------------------------

	Entity* EntityManager::GetRawEntity( const u32& id )
	{
		if ( id < MAX_ENTITIES && mEntities.at( id ).mState != EntityState::INVALID )
		{
			return &mEntities.at( id );
		}

		return nullptr;
	}

	//---------------------------------------------------------------

	void EntityManager::Destroy( const EntityHandle& entity )
	{
		if ( !entity.Get( ) || entity.Get()->mIsArchetypeRoot )
		{
			return;
		}

		// Destroy all children as well
		for ( auto& e : entity.Get( )->GetChildren( ) )
		{
			// Destroy entity
			e.Get()->Destroy( ); 
		}

		// Push for deferred removal from active entities
		mMarkedForDestruction.push_back( entity.GetID( ) );

		// Set entity to be invalid
		entity.Get( )->mState = EntityState::INACTIVE;

		// Remove from need initialization lists
		RemoveFromNeedInitLists( entity );

		// Remove from need start lists
		RemoveFromNeedStartLists( entity );
	}

	//==============================================================================

	void EntityManager::RemoveFromNeedInitLists( const EntityHandle& entity )
	{
		Entity* ent = entity.Get( );
		if ( ent )
		{
			// Remove all components
			for ( auto& c : ent->GetComponents( ) )
			{
				mNeedInitializationList.erase( std::remove( mNeedInitializationList.begin( ), mNeedInitializationList.end( ), c ), mNeedInitializationList.end( ) );
			}
		}
	}

	//==============================================================================

	void EntityManager::RemoveFromNeedStartLists( const EntityHandle& entity )
	{
		Entity* ent = entity.Get( );
		if ( ent )
		{
			// Remove all components
			for ( auto& c : ent->GetComponents( ) )
			{
				mNeedStartList.erase( std::remove( mNeedStartList.begin( ), mNeedStartList.end( ), c ), mNeedStartList.end( ) );
			}
		}
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
		mNeedInitializationList.clear( );
		mNeedStartList.clear( );
	}

	//==============================================================================

	World* EntityManager::GetArchetypeWorld( ) const
	{
		return mArchetypeWorld;
	}

	//==============================================================================

	const HashSet< Entity* >& EntityManager::GetEntitiesByWorld( const World* world )
	{
		// World must be registerd!
		assert( WorldExists( world ) );

		return mWorldEntityMap[ world ];
	}

	//==============================================================================

	void EntityManager::ForceCleanup( )
	{
		Cleanup( );

		mNeedInitializationList.clear( );
		mNeedStartList.clear( );
	}

	//==============================================================================

	void EntityManager::ForceAddEntities( )
	{
		// Add all new entities into active entities
		for ( auto& e : mMarkedForAdd )
		{
			mActiveEntities.push_back( e );
		}

		// Clear the marked for add entities
		mMarkedForAdd.clear( );
	}

	//==============================================================================

	void EntityManager::Cleanup( )
	{
		// Move through dirty list and remove from active entities
		for ( auto& e : mMarkedForDestruction )
		{
			if ( e < MAX_ENTITIES )
			{
				Entity* ent = &mEntities.at( e );

				if ( ent && ent->mState != EntityState::INVALID )
				{
					// Destroy all components
					for ( auto& c : ent->mComponents )
					{
						auto comp = GetComponent( ent->GetHandle( ), c );
						if ( comp )
						{
							// Call shutdown on component
							comp->Shutdown( );
							// Destroy the component
							comp->Destroy( );
							// Remove component from world
							comp->RemoveFromWorld( );
						}

						// Free component memory
						delete comp;
						// Set to null
						comp = nullptr;
					}

					// Remove entity ( includes reset )
					RemoveEntityUnsafe( ent );
				}
			}
		}

		mMarkedForDestruction.clear( );
	}

	//==================================================================================================

	void EntityManager::RemoveEntityUnsafe( Entity* entity )
	{
		// Reset the entity
		entity->Reset( );

		// Remove from active entities
		mActiveEntities.erase( std::remove( mActiveEntities.begin( ), mActiveEntities.end( ), entity ), mActiveEntities.end( ) ); 
	}

	//==================================================================================================

	void EntityManager::AddEntityToWorld( Entity* entity, World* world )
	{ 
		if ( world && entity ) 
		{
			auto query = mWorldEntityMap.find( world );
			if ( query != mWorldEntityMap.end( ) )
			{
				if ( mWorldEntityMap[ world ].find( entity ) == mWorldEntityMap[ world ].end( ) )
				{
					mWorldEntityMap[ world ].insert( entity ); 

					// Add children to world
					for ( auto& c : entity->GetChildren( ) )
					{
						AddEntityToWorld( c.Get( ), world );
					}

					// Add components to world
					for ( auto& c : entity->GetComponents( ) )
					{
						c->AddToWorld( world );
					}

					entity->mWorld = world; 
				}
			}
		}
	}

	//==================================================================================================

	void EntityManager::RemoveEntityFromWorld( Entity* entity )
	{
		const World* world = entity->GetWorld( );
		if ( world )
		{
			auto query = mWorldEntityMap.find( world );
			if ( query != mWorldEntityMap.end( ) )
			{
				// Erase entity from list if found
				HashSet< Entity* >* ents = &mWorldEntityMap[ world ];
				ents->erase( entity );
				//ents->erase( std::remove( ents->begin( ), ents->end( ), entity ), ents->end( ) );

				// Remove children from world as well
				for ( auto& c : entity->GetChildren( ) )
				{
					RemoveEntityFromWorld( c.Get( ) );
				}

				// Remove components from world
				for ( auto& c : entity->GetComponents( ) )
				{
					c->RemoveFromWorld( );
				}

				// Set world to null
				entity->mWorld = nullptr;
			}
		}
	}

	//==================================================================================================

	void EntityManager::RemoveWorld( const World* world )
	{
		auto query = mWorldEntityMap.find( world );
		if ( query != mWorldEntityMap.end( ) )
		{
			// Destroy all entities in list
			HashSet< Entity* >& ents = mWorldEntityMap[ world ];
			for ( auto& e : ents )
			{
				e->Destroy( );
			}

			// Clear list
			ents.clear( );
		}

		// Remove world from map
		mWorldEntityMap.erase( world );
	}

	//==================================================================================================

	Result EntityManager::Initialize( )
	{
		// Set all components to null
		for ( auto i = 0; i < mComponents.size( ); i++ )
		{
			mComponents.at( i ) = nullptr;
		}

		// Reset available id and then resize entity storage array
		mNextAvailableID = 0;
		mEntities.resize( MAX_ENTITIES ); 

		// Register all engine level components with component array 
		RegisterAllEngineComponents( );

		// Construct archetype world and register contexts
		mArchetypeWorld = new World( );
		mArchetypeWorld->RegisterContext< EntitySubsystemContext >( );
		mArchetypeWorld->RegisterContext< GraphicsSubsystemContext >( );
		mArchetypeWorld->SetShouldUpdate( false );
		// Set their updates to false
		mArchetypeWorld->SetUpdates< EntitySubsystemContext >( false );
		mArchetypeWorld->SetUpdates< GraphicsSubsystemContext >( false );

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
			if ( e->mState == EntityState::ACTIVE )
			{
				// Push back entity
				mActiveEntities.push_back( e );

				// Push back entity into its world map vector
				AddEntityToWorld( e, e->GetWorld( )->ConstCast< World >( ) ); 
			} 
		}

		// Clear the marked for add entities
		mMarkedForAdd.clear( );

		// If the application is running 
		if ( Engine::GetInstance( )->GetApplication( )->GetApplicationState( ) == ApplicationState::Running )
		{
			// Process all components that need initialization from last frame
			for ( auto& c : mNeedInitializationList )
			{
				if ( c )
				{
					c->Initialize( );
				}
			}

			// Process all components that need startup from last frame 
			for ( auto& c : mNeedStartList )
			{
				if ( c )
				{
					c->Start( );
				}
			}

			// Clear both lists
			mNeedInitializationList.clear( );
			mNeedStartList.clear( );
		}

		// Update all component systems
		for ( auto& system : mComponents )
		{
			system.second->Update( );
		}
	}

	//==================================================================================================

	void EntityManager::LateUpdate( f32 dt )
	{
		// Clean any entities that were marked for destruction
		UpdateAllActiveTransforms( dt );
	}

	//==================================================================================================

	Result EntityManager::Shutdown( )
	{
		// Destroy world
		if ( mArchetypeWorld )
		{
			delete ( mArchetypeWorld );
			mArchetypeWorld = nullptr;
		}

		// Detach all components from entities
		for ( u32 i = 0; i < MAX_ENTITIES; ++i )
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

		// Clear all lists to free memory
		mNeedStartList.clear( );
		mActiveEntities.clear( );
		mNeedInitializationList.clear( );
		mMarkedForAdd.clear( );
		mMarkedForDestruction.clear( );

		return Result::SUCCESS;
	}

	//================================================================================================== 

	void EntityManager::RegisterAllEngineComponents( )
	{
		// Register engine components here
		RegisterComponent< StaticMeshComponent >( );
		RegisterComponent< PointLightComponent >( );
		RegisterComponent< RigidBodyComponent >( );
		RegisterComponent< DirectionalLightComponent >( );
		RegisterComponent< CameraComponent >( );
		RegisterComponent< SkeletalMeshComponent >( );
		RegisterComponent< SkeletalAnimationComponent >( );
	}

	//================================================================================================== 

	void EntityManager::UpdateAllActiveTransforms( f32 dt )
	{
		// Does nothing for now
	}

	//========================================================================================================================

	void EntityManager::RegisterComponent( const MetaClass* cls )
	{
		u32 index = cls->GetTypeId( );
		mComponents[ index ] = new ComponentArray( );
	}

	//========================================================================================================================

	// TODO(): Need to have a destinction here on whether or not the component being 
	// asked to unregistered is an engine-level component or not - Most likely NEVER want to be able to 
	// unregister one of those
	void EntityManager::UnregisterComponent( const MetaClass* cls )
	{
		u32 index = cls->GetTypeId( );

		// For now will only erase if there are no components attached to any entities
		if ( ComponentBaseExists( index ) && mComponents[ index ]->IsEmpty( ) )
		{
			ComponentWrapperBase* base = mComponents[ index ];
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
		assert( entity != nullptr );

		// Doesn't have component
		if ( !ComponentBaseExists( compIdx ) )
		{
			RegisterComponent( compCls );
		}

		assert( mComponents.at( compIdx ) != nullptr );

		// Entity id
		u32 eid = entity->GetID( );

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
			component->SetEntity( entity );
			component->SetID( compIdx );
			component->SetBase( base );
			component->mEntityID = entity->mID;
			component->PostConstruction( );

			// Get component ptr and push back into entity components
			entity->mComponents.push_back( compIdx );

			// Push back for need initilization and start
			mNeedInitializationList.push_back( component );
			mNeedStartList.push_back( component );

			// Need to add required components from meta class
			const MetaClassComponent* cc = static_cast<const MetaClassComponent*>( compCls );
			if ( cc )
			{
				for ( auto& c : cc->GetRequiredComponentList( ) )
				{
					const MetaClass* cls = Object::GetClass( c );
					if ( cls )
					{
						// Add component if entity doesn't already contain this one
						if ( !entity->HasComponent( cls ) )
						{
							AddComponent( cls, entity );
						}
					}
				}
			}
		}

		return component;
	}

	//=========================================================================================

	void EntityManager::RemoveComponent( const MetaClass* compCls, const EntityHandle& entity )
	{
		auto comp = GetComponent( entity, compCls->GetTypeId( ) );
		if ( comp )
		{
			// Remove from initialization list
			mNeedInitializationList.erase( std::remove( mNeedInitializationList.begin( ), mNeedInitializationList.end( ), comp ), mNeedInitializationList.end( ) );

			// Remove from start list
			mNeedStartList.erase( std::remove( mNeedStartList.begin( ), mNeedStartList.end( ), comp ), mNeedStartList.end( ) );

			// Destroy component
			comp->Destroy( );

			// Remove from entity component list
			auto comps = &entity.Get( )->mComponents;
			comps->erase( std::remove( comps->begin( ), comps->end( ), compCls->GetTypeId( ) ), comps->end( ) );

			// Free memory of component
			delete comp;
			// Set to null
			comp = nullptr;
		}
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

	void EntityManager::RecurisvelyGenerateNewUUIDs( const EntityHandle& entity )
	{
		Entity* ent = entity.Get( );
		if ( !ent )
		{
			return;
		}

		// Remove entity from map
		RemoveFromUUIDMap( ent );

		// Construct new UUID for entity
		ent->mUUID = UUID::GenerateUUID( );

		// Add to uuid map with new id
		AddToUUIDMap( ent ); 

		// Continue for each child
		for ( auto& c : ent->GetChildren( ) )
		{
			RecurisvelyGenerateNewUUIDs( c );
		}
	}

	//=========================================================================================

	void EntityManager::RecursivelySetPrototypeEntities( const EntityHandle& source, const EntityHandle& dest )
	{
		Entity* destEnt = dest.Get( );
		Entity* sourceEnt = source.Get( );

		Vector< EntityHandle > destChildren = destEnt->GetChildren( );
		Vector< EntityHandle > sourceChildren = sourceEnt->GetChildren( );

		if ( destChildren.size( ) != sourceChildren.size( ) )
		{
			// Error...
			return;
		}

		// Set prototype entity to the source entity
		destEnt->SetPrototypeEntity( sourceEnt );

		// For all children, recursively set prototype entities
		for ( usize i = 0; i < destChildren.size(); ++i )
		{
			RecursivelySetPrototypeEntities( sourceChildren.at( i ), destChildren.at( i ) );
		}
	}

	//=========================================================================================

	EntityHandle EntityManager::CopyEntity( const EntityHandle& entity, World* world )
	{
		if ( world == nullptr )
		{
			world = Engine::GetInstance( )->GetWorld( );
		}

		// Use to serialize entity data for new entity
		ByteBuffer buffer;

		// Set up the handle using the other
		if ( entity.Get( ) )
		{
			// Get entities
			Entity* sourceEnt = entity.Get( );

			// Serialize entity into buffer
			EntityArchiver::Serialize( entity, &buffer );

			// Deserialize into new entity
			EntityHandle newHandle = EntityArchiver::Deserialize( &buffer, world );

			// Destination entity
			Entity* destEnt = newHandle.Get( );

			// Ensure that all UUIDs are unique
			RecurisvelyGenerateNewUUIDs( destEnt ); 
 
			// Cache off local transform of destination entity before parenting
			Transform localTrans = destEnt->GetLocalTransform( );

			// Set parent of new handle
			if ( sourceEnt->GetParent( ) )
			{
				sourceEnt->GetParent( ).Get( )->AddChild( destEnt );

				// Reset local transform
				destEnt->SetLocalTransform( localTrans );
			}

			// Return the handle, valid or not
			return newHandle;
		}

		// Return empty handle
		return EntityHandle( );
	}

	//=========================================================================================

	EntityHandle EntityManager::InstanceEntity( const EntityHandle& entity, World* world )
	{
		if ( world == nullptr )
		{
			world = Engine::GetInstance( )->GetWorld( );
		}

		// Use to serialize entity data for new entity
		ByteBuffer buffer;

		// Set up the handle using the other
		if ( entity.Get( ) )
		{
			// Get entities
			Entity* sourceEnt = entity.Get( );

			// Serialize entity into buffer
			EntityArchiver::Serialize( entity, &buffer );

			// Deserialize into new entity
			EntityHandle newHandle = EntityArchiver::Deserialize( &buffer, world );

			// Destination entity
			Entity* destEnt = newHandle.Get( );

			// Construct new UUID for entity
			destEnt->mUUID = UUID::GenerateUUID( );

			// Remove from previous prototype if available
			if ( destEnt->HasPrototypeEntity( ) )
			{
				destEnt->RemovePrototypeEntity( );
			}

			// Ensure that all UUIDs are unique
			for ( auto& c : destEnt->GetChildren( ) )
			{
				RecurisvelyGenerateNewUUIDs( c );
			}

			// Set all prototype entities
			RecursivelySetPrototypeEntities( sourceEnt, destEnt );

			// Return the handle, valid or not
			return newHandle;
		}

		// Return empty handle
		return EntityHandle( );
	}

	//========================================================================================= 

	EntitySubsystemContext::EntitySubsystemContext( World* world )
		: SubsystemContext( world )
	{
		// Add world to entity manager
		EngineSubsystem( EntityManager )->AddWorld( mWorld );
	}

	//=========================================================================================

	void EntitySubsystemContext::ExplicitDestructor( )
	{
		// Remove from entity world
		EngineSubsystem( EntityManager )->RemoveWorld( mWorld );
	}

	//========================================================================================= 
}


/*

// Archetypes...

// How are these going to be constructed?

Entities can be instantiated from archetypes

ENJON_CLASS( )
class Archetype : public Asset
{
ENJON_CLASS_BODY( Archetype )
};


*/
















