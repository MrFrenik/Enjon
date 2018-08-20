// File: EntityManager.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#ifndef ENJON_ENTITY_MANAGER_H
#define ENJON_ENTITY_MANAGER_H
#pragma once

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityDefines.h"
#include "Math/Transform.h" 
#include "Base/Object.h"
#include "System/Types.h"
#include "Serialize/UUID.h"
#include "Defines.h"
#include "Subsystem.h"
#include "SubsystemCatalog.h"
#include "Base/SubsystemContext.h"
#include "Entity/Archetype.h"
#include "Asset/Asset.h"
#include "Engine.h"

#include <array>
#include <vector>
#include <cstdint>
#include <assert.h>

namespace Enjon
{
	class EntityManager;
	class EntityArchiver;
	class World;

	enum class EntityState
	{
		INACTIVE,
		ACTIVE,
		INVALID
	};

	// Forward declaration
	class Entity;
	class EntityManager;

	ENJON_CLASS( )
	class EntitySubsystemContext : public SubsystemContext
	{
		ENJON_CLASS_BODY( EntitySubsystemContext )

		public:
			/**
			* @brief
			*/
			EntitySubsystemContext( World* world );

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( );

		protected:

		private:
	};

	class EntityHandle
	{
		friend Application;
		friend EntityManager;

	public:
		/*
		* @brief Constructor
		*/
		EntityHandle( );

		/*
		* @brief Constructor
		*/
		EntityHandle( const Entity* entity );

		/*
		* @brief Destructor
		*/
		~EntityHandle( );

		/*
		* @brief
		*/
		u32 GetID( ) const;

		/*
		* @brief
		*/
		friend bool operator==( EntityHandle left, const EntityHandle& other );

		/*
		* @brief
		*/
		static EntityHandle Invalid( );

		/*
		* @brief
		*/
		bool IsValid( ) const;

		/*
		* @brief
		*/
		explicit operator bool( );

		/*
		* @brief
		*/
		Enjon::Entity* Get( ) const;

	private:
		u32 mID = MAX_ENTITIES;
	};

	ENJON_CLASS( )
	class Entity : public Enjon::Object
	{
		ENJON_CLASS_BODY( Entity )

		friend EntityHandle;
		friend EntityManager;
		friend EntityArchiver;
		friend Archetype;

	public:

		/**
		* @brief
		*/
		virtual void ExplicitConstructor( ) override;

		/**
		* @brief
		*/
		virtual void ExplicitDestructor( );

		/**
		* @brief Get id of this entity
		*/
		u32 GetID( ) const;

		/**
		* @brief Get uuid of this entity
		*/
		UUID GetUUID( ) const;

		/**
		* @brief Get name of this entity
		*/
		String GetName( ) const;

		/*
		* @brief
		*/
		void SetName( const String& uuid );

		/*
		* @brief
		*/
		const World* GetWorld( ) const;

		/**
		* @brief
		*/
		virtual Result OnEditorUI( ) override;
 
		/**
		* @brief
		*/
		virtual Result MergeWith( Object* source, MergeType mergeType ) override;

		/**
		* @brief
		*/
		virtual Result RecordPropertyOverrides( Object* source ) override;

		/**
		* @brief
		*/
		virtual Result ClearAllPropertyOverrides( ) override;

		/**
		* @brief Checks whether or not entity has given component
		*/
		template <typename T>
		bool HasComponent( );

		/**
		* @brief
		*/
		bool HasComponent( const MetaClass* compCls );

		/**
		* @brief Gets component from entity, if exists
		*/
		template <typename T>
		T* GetComponent( );

		/*
		* @brief
		*/
		Component* GetComponent( const MetaClass* compCls );

		/**
		* @brief Attaches component to entity, if exists
		*/
		template <typename T>
		T* AddComponent( );

		/*
		* @brief
		*/
		Component* AddComponent( const MetaClass* compCls );

		/**
		* @brief Detaches component from entity, if exists
		*/
		template <typename T>
		void RemoveComponent( );

		/**
		* @brief Detaches component from entity, if exists
		* @note NOT ITERABLE ( Will remove from the list of existing components )
		*/
		void RemoveComponent( const MetaClass* cls );

		/**
		* @brief Sets local transform of entity
		*/
		void SetLocalTransform( const Transform& transform, bool propagateToComponents = true );

		/**
		* @brief
		*/
		AssetHandle< Archetype > GetArchetype( ) const;

		/**
		* @brief Gets local transform of entity relative to parent entity, if exists
		*/
		Transform GetLocalTransform( );

		/**
		* @brief Gets World transform of entity which calculates world transform if dirty flag is set
		*/
		Transform GetWorldTransform( );

		/**
		* @brief Sets local position of entity relative to parent, if exists
		*/
		void SetLocalPosition( Vec3& position, bool propagateToComponents = true );

		/**
		* @brief Sets local scale of entity relative to parent, if exists
		*/
		void SetLocalScale( Vec3& scale, bool propagateToComponents = true );

		/**
		* @brief Sets local scale of entity relative to parent, if exists
		*/
		void SetLocalScale( f32 scale, bool propagateToComponents = true );

		/**
		* @brief Sets local orientation of entity relative to parent, if exists
		*/
		void SetLocalRotation( Quaternion& rotation, bool propagateToComponents = true );

		/**
		* @brief Sets local orientation of entity relative to parent, if exists
		*/
		void SetLocalRotation( Vec3& eulerAngles, bool propagateToComponents = true );

		/**
		* @brief Gets local position of entity relative to parent, if exists
		*/
		Vec3 GetLocalPosition( );

		/**
		* @brief Gets local scale of entity relative to parent, if exists
		*/
		Vec3 GetLocalScale( );

		/**
		* @brief Gets local rotation of entity relative to parent, if exists
		*/
		Quaternion GetLocalRotation( );

		/**
		* @brief Gets World position of entity which calculates world transform if dirty flag is set
		*/
		ENJON_FUNCTION( )
		Vec3 GetWorldPosition( );

		/**
		* @brief Gets World scale of entity which calculates world transform if dirty flag is set
		*/
		Vec3 GetWorldScale( );

		/**
		* @brief Gets World rotation of entity which calculates world transform if dirty flag is set
		*/
		Quaternion GetWorldRotation( );

		/**
		* @brief Gets parent of this entity, returns nullptr if doesn't exist
		*/
		EntityHandle GetParent( ) { return mParent; }

		/**
		* @brief
		*/
		bool ExistsInChildHierarchy( const EntityHandle& child );

		/**
		* @brief Registers a child with this entity
		*/
		void AddChild( const EntityHandle& child );

		/**
		* @brief Removes child from entity, if exists
		*/
		void DetachChild( const EntityHandle& child, bool deferRemovalFromList = false );

		/**
		* @brief Sets parent of entity, if one doesn't already exist
		*/
		void SetParent( const EntityHandle& parent );

		/**
		* @brief Removes parent from entity, if one exists
		*/
		void RemoveParent( bool deferRemovalFromList = false );

		/**
		* @brief
		*/
		EntityHandle GetHandle( );

		/**
		* @brief Returns whether or not has prototype entity
		*/
		bool HasPrototypeEntity( ) const;

		/**
		* @brief
		*/
		EntityHandle GetPrototypeEntity( ) const;

		/**
		* @brief Returns whether or not has parent
		*/
		b8 HasParent( ); 

		/**
		* @brief Returns whether or not has children
		*/
		b8 HasChildren( );

		/**
		* @brief
		*/
		b8 HasChild( const EntityHandle& child );

		/**
		* @brief Returns whether or not entity is valid
		*/
		b8 IsValid( );

		/*
		* @brief
		*/
		Vec3 Forward( );

		/*
		* @brief
		*/
		Vec3 Right( );

		/*
		* @brief
		*/
		Vec3 Up( );

		/**
		* @brief
		*/
		//const Vector< EntityHandle >& GetChildren() const { return mChildren; }

		/**
		* @brief
		*/
		Vector< EntityHandle > GetChildren( );

		/**
		* @brief
		*/
		Vector< EntityHandle > GetInstancedEntities( );

		/**
		* @brief
		*/
		const Vector<u32>& GetComponentIndicies( ) const
		{
			return mComponents;
		}

		/**
		* @brief
		*/
		Vector<Component*> GetComponents( );

		/**
		* @brief Propagates transform down through all components
		*/
		void UpdateComponentTransforms( );

		/*
		* @brief
		*/
		void Destroy( ); 

		/*
		* @brief
		*/
		void MoveToWorld( World* world );

		/*
		* @brief
		*/
		const EntityState& GetState( ) const;

	protected:

		/*
		* @brief
		*/
		void Update( const f32& dt );

		/*
		* @brief
		*/
		void CalculateLocalTransform( );

		/*
		* @brief
		*/
		void CalculateWorldTransform( );

		/*
		* @brief
		*/
		void SetUUID( const UUID& uuid );

		/*
		* @brief
		*/
		void RemoveFromWorld( );

		/*
		* @brief
		*/
		void SetArchetype( const AssetHandle< Archetype >& archType );

		/*
		* @brief
		*/
		void SetPrototypeEntity( const EntityHandle& handle );

		/*
		* @brief
		*/
		void RemovePrototypeEntity( );

		/*
		* @brief
		*/
		void AddInstance( const EntityHandle& handle );

		/*
		* @brief
		*/
		void RemoveInstance( const EntityHandle& handle );

	private:
		/*
		* @brief
		*/
		void SetID( u32 id );

		/*
		* @brief
		*/
		void Reset( );

		/*
		* @brief
		*/
		void SetAllChildWorldTransformsDirty( );

		/*
		* @brief
		*/
		void UpdateAllChildTransforms( );

		/*
		* @brief
		*/
		void PropagateTransform( f32 dt ); 

	private:
		ENJON_PROPERTY( NonSerializeable, ReadOnly )
		u32 mID = MAX_ENTITIES;

		ENJON_PROPERTY( NonSerializeable, ReadOnly )
		EntityHandle mParent;
 
		ENJON_PROPERTY( NonSerializeable, ReadOnly )
		Transform mLocalTransform;

		ENJON_PROPERTY( NonSerializeable, ReadOnly )
		Transform mWorldTransform;

		ENJON_PROPERTY( NonSerializeable, HideInEditor )
		Vector<u32> mComponents;

		ENJON_PROPERTY( NonSerializeable )
		Vector< EntityHandle > mChildren;

		ENJON_PROPERTY( NonSerializeable, HideInEditor, ReadOnly )
		HashSet< u32 > mInstancedEntities;

		ENJON_PROPERTY( NonSerializeable, ReadOnly )
		UUID mUUID;

		ENJON_PROPERTY( NonSerializeable )
		String mName = "Entity";

		ENJON_PROPERTY( NonSerializeable, ReadOnly )
		EntityHandle mPrototypeEntity;

		ENJON_PROPERTY( NonSerializeable, ReadOnly, HideInEditor )
		AssetHandle< Archetype > mArchetype; 

		const World* mWorld = nullptr;

		Enjon::EntityState mState;

		u32 mWorldTransformDirty : 1; 
		u32 mIsArchetypeRoot : 1;
	};

	//using EntityStorage 			= std::array<Entity, MAX_ENTITIES>*;
	using EntityStorage = Vector< Entity >;
	using MarkedForDestructionList = Vector< u32 >;
	using EntityList = Vector<Entity*>;
	using ComponentBaseArray = HashMap< u32, ComponentWrapperBase* >;
	using ComponentList = Vector< Component* >;
	using WorldEntityMap = HashMap< const World*, HashSet< Entity* > >;

	ENJON_CLASS( )
		class EntityManager : public Subsystem
	{
		ENJON_CLASS_BODY( EntityManager )

			friend Entity;
		friend Application;
		friend World;
		friend EntitySubsystemContext;

	public:

		/*
		* @brief
		*/
		Enjon::EntityHandle Allocate( World* world = nullptr );

		/**
		*@brief
		*/
		virtual Enjon::Result Initialize( ) override;

		/**
		* @brief
		*/
		Entity* GetRawEntity( const u32& id );

		/**
		*@brief
		*/
		virtual void Update( const f32 dT ) override;

		/*
		* @brief
		*/
		void LateUpdate( f32 dt );

		/**
		*@brief
		*/
		virtual Enjon::Result Shutdown( ) override;

		/**
		*@brief
		*/
		template <typename T>
		void RegisterComponent( );

		/**
		*@brief
		*/
		void RegisterComponent( const MetaClass* cls );

		/**
		*@brief
		*/
		void UnregisterComponent( const MetaClass* cls );

		/**
		*@brief
		*/
		Vector<const MetaClass*> GetComponentMetaClassList( );

		/**
		*@brief
		*/
		template <typename T>
		T* AddComponent( const Enjon::EntityHandle& entity );

		/**
		*@brief
		*/
		Component* AddComponent( const MetaClass* compCls, const Enjon::EntityHandle& handle );

		/**
		*@brief
		*/
		template <typename T>
		void RemoveComponent( Entity* entity );

		/**
		*@brief
		*/
		void RemoveComponent( const MetaClass* compCls, const EntityHandle& entity );

		/**
		*@brief
		*/
		template <typename T>
		T* GetComponent( Entity* entity );

		/**
		*@brief
		*/
		Component* GetComponent( const MetaClass* compCls, const EntityHandle& entity );

		/**
		*@brief
		*/
		EntityHandle CopyEntity( const EntityHandle& entity, World* world = nullptr );

		/**
		*@brief
		*/
		EntityHandle InstanceEntity( const EntityHandle& entity, World* world = nullptr );

		/**
		*@brief
		*/
		void RecurisvelyGenerateNewUUIDs( const EntityHandle& entity );

		/**
		*@brief
		*/
		void RecursivelySetPrototypeEntities( const EntityHandle& source, const EntityHandle& dest );

		/**
		*@brief
		*/
		void Destroy( const EntityHandle& entity );


		/**
		* @brief Returns vector of active entities that have no parent
		*/
		Vector<EntityHandle> GetRootLevelEntities( World* world = nullptr );

		/**
		* @brief Returns vector of active components of given type
		*/
		template <typename T>
		Vector<Component*> GetAllComponentsOfType( );

		/**
		*@brief
		*/
		const Vector<Entity*>& GetActiveEntities( ) const
		{
			return mActiveEntities;
		}

		/**
		*@brief
		*/
		const HashSet< Entity* >& GetEntitiesByWorld( const World* world );

		/**
		*@brief
		*/
		World* GetArchetypeWorld( ) const;

		/*
		* @brief
		*/
		void ForceCleanup( );

		/*
		* @brief
		*/
		void ForceAddEntities( );

		/**
		* @brief
		*/
		bool ComponentBaseExists( const u32& compIdx );

		/**
		* @brief Destroys all entities - both active and pending( will clear all lists, so is not iterable safe )
		* @note NOT SAFE TO CALL DURING ANY ENTITY UPDATE LOOP (Only call within application update)
		*/
		void DestroyAll( );

		/**
		* @brief
		*/
		EntityHandle GetEntityByUUID( const UUID& uuid );

	protected:

		/**
		* @brief
		*/
		void RegisterAllEngineComponents( );


		/**
		* @brief
		*/
		Component* GetComponent( const EntityHandle& entity, const u32& ComponentID );

		/**
		* @brief
		*/
		void RemoveWorld( const World* world );

		/**
		* @brief
		*/
		bool WorldExists( const World* world );

		/**
		* @brief
		*/
		void AddWorld( const World* world );

	private:
		/**
		*@brief
		*/
		void Cleanup( );

		/**
		*@brief
		*/
		void RemoveEntityUnsafe( Entity* entity );

		/**
		*@brief
		*/
		void RemoveEntityFromWorld( Entity* entity );

		/**
		*@brief
		*/
		void AddEntityToWorld( Entity* entity, World* world );

		/**
		*@brief
		*/
		static b8 CompareEntityIDs( const Entity* a, const Entity* b );

		/**
		*@brief
		*/
		template <typename T>
		void DetachComponentFromEntity( Entity* entity );

		/**
		*@brief
		*/
		u32 FindNextAvailableID( );

		/**
		*@brief
		*/
		void UpdateAllActiveTransforms( f32 dt );

		/**
		*@brief
		*/
		void RemoveFromNeedInitLists( const EntityHandle& entity );

		/**
		*@brief
		*/
		void RemoveFromNeedStartLists( const EntityHandle& entity );

		/**
		*@brief
		*/
		template <typename T >
		bool ComponentBaseExists( );

	private:
		EntityStorage 				mEntities;
		ComponentBaseArray 			mComponents;
		EntityList 					mActiveEntities;
		EntityList 					mMarkedForAdd;
		MarkedForDestructionList 	mMarkedForDestruction;
		ComponentList				mNeedInitializationList;
		ComponentList				mNeedStartList;
		u32 						mNextAvailableID = 0;
		WorldEntityMap				mWorldEntityMap;
		World*						mArchetypeWorld = nullptr;
	};

#include "Entity/Entity.inl"
#include "Entity/EntityManager.inl"
}

#endif


