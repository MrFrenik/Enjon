#ifndef ENJON_ENTITY_MANAGER_H
#define ENJON_ENTITY_MANAGER_H
#pragma once

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityDefines.h"
#include "Math/Transform.h" 
#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Subsystem.h"

#include <array>
#include <vector>
#include <cstdint>
#include <assert.h>

namespace Enjon 
{ 
	class EntityManager;

	enum class EntityState
	{
		INACTIVE,
		ACTIVE
	};

	// Forward declaration
	class Entity;
	class EntityManager;

	class EntityHandle
	{
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
		friend bool operator==( EntityHandle left, const EntityHandle& other);

		/*
		* @brief
		*/
		Enjon::Entity* Get( ) const;

	private:
		const Enjon::Entity* mEntity = nullptr;
		u32 mID = MAX_ENTITIES;
	};

	ENJON_CLASS( )
	class Entity : public Enjon::Object
	{
		ENJON_CLASS_BODY( Entity )

		friend EntityHandle;
		friend EntityManager; 

		public:
			Entity();
			Entity(EntityManager* manager);
			~Entity();

			/// @brief Get id of this entity
			u32 GetID() { return mID; }

			/// @brief Checks whether or not entity has given component
			template <typename T>
			bool HasComponent();

			bool HasComponent( const MetaClass* compCls );

			/// @brief Gets component from entity, if exists
			template <typename T>
			T* GetComponent();

			/// @brief Attaches component to entity, if exists
			template <typename T>
			T* AddComponent();

			/*
			* @brief
			*/
			Component* AddComponent( const MetaClass* compCls );

			/// @brief Detaches component from entity, if exists
			template <typename T>
			void RemoveComponent();

			/// @brief Sets local transform of entity
			void SetLocalTransform(Transform& transform);

			/// @brief Gets local transform of entity relative to parent entity, if exists
			Transform GetLocalTransform();

			/// @brief Gets World transform of entity which calculates world transform if dirty flag is set
			Transform GetWorldTransform();

			/// @brief Sets local position of entity relative to parent, if exists
			ENJON_FUNCTION( )
			void SetPosition(Vec3& position);

			/// @brief Sets local scale of entity relative to parent, if exists
			void SetScale(Vec3& scale);

			/// @brief Sets local scale of entity relative to parent, if exists
			void SetScale(f32 scale);

			/// @brief Sets local orientation of entity relative to parent, if exists
			void SetRotation(Quaternion& rotation);

			/// @brief Gets local position of entity relative to parent, if exists
			Vec3 GetLocalPosition();

			/// @brief Gets local scale of entity relative to parent, if exists
			Vec3 GetLocalScale();

			/// @brief Gets local rotation of entity relative to parent, if exists
			Quaternion GetLocalRotation();

			/// @brief Gets World position of entity which calculates world transform if dirty flag is set
			ENJON_FUNCTION( )
			Vec3 GetWorldPosition();

			/// @brief Gets World scale of entity which calculates world transform if dirty flag is set
			Vec3 GetWorldScale();

			/// @brief Gets World rotation of entity which calculates world transform if dirty flag is set
			Quaternion GetWorldRotation();

			/// @brief Gets parent of this entity, returns nullptr if doesn't exist
			EntityHandle GetParent() { return mParent; }

			/// @brief Registers a child with this entity
			void AddChild(const EntityHandle& child);

			/// @brief Removes child from entity, if exists
			void DetachChild(const EntityHandle& child, bool deferRemovalFromList = false );	

			/// @brief Sets parent of entity, if one doesn't already exist
			void SetParent(const EntityHandle& parent);

			/// @brief Removes parent from entity, if one exists
			void RemoveParent( bool deferRemovalFromList = false );
			
			/// @brief 
			EntityHandle GetHandle( );

			/// @brief Returns whether or not has parent
			b8 Entity::HasParent();

			/// @brief Returns whether or not has children
			b8 Entity::HasChildren();

			/// @brief Returns whether or not entity is valid
			b8 Entity::IsValid();

			const Vector< EntityHandle >& GetChildren() const { return mChildren; }

			const Vector<u32>& GetComponentIndicies( ) const 
			{ 
				return mComponents; 
			}

			Vector<Component*> GetComponents( ); 

			/// @brief Propagates transform down through all components
			void UpdateComponentTransforms(f32 dt);

			/*
			* @brief
			*/
			void Destroy( );

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
			void CalculateWorldTransform();

		private:
			/*
			* @brief
			*/
			void SetID(u32 id);

			/*
			* @brief
			*/
			void Reset();

			/*
			* @brief
			*/
			void SetAllChildWorldTransformsDirty();

			/*
			* @brief
			*/
			void UpdateAllChildTransforms();

			/*
			* @brief
			*/
			void PropagateTransform(f32 dt); 

		private:
			ENJON_PROPERTY( )
			u32 mID = MAX_ENTITIES;	

			u32 mWorldTransformDirty : 1; 					

			ENJON_PROPERTY( )
			EntityHandle mParent;

			ENJON_PROPERTY( )
			Transform mLocalTransform;

			ENJON_PROPERTY( )
			Transform mWorldTransform;

			ENJON_PROPERTY( )
			Vector<u32> mComponents;
 
			ENJON_PROPERTY( )
			Vector< EntityHandle > mChildren; 

			Enjon::ComponentBitset mComponentMask;
			Enjon::EntityManager* mManager;
			Enjon::EntityState mState;
			EntityHandle mHandle;
	};

	using EntityStorage 			= std::array<Entity, MAX_ENTITIES>*;
	using MarkedForDestructionList	= Vector< u32 >;
	using ActiveEntityList 			= Vector<Entity*>;
	using ComponentBaseArray		= HashMap< u32, ComponentWrapperBase* >;

	ENJON_CLASS( )
	class EntityManager : public Subsystem
	{
		ENJON_CLASS_BODY( )

		friend Entity;
		public:

			/*
			* @brief
			*/
			EntityManager();

			/*
			* @brief
			*/
			~EntityManager();
			
			/*
			* @brief
			*/
			Enjon::EntityHandle Allocate( );

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
			void LateUpdate(f32 dt);

			/**
			*@brief
			*/
			virtual Enjon::Result Shutdown( ) override; 


			/**
			*@brief
			*/
			template <typename T>
			void RegisterComponent();

			/**
			*@brief
			*/
			template <typename T>
			Vector<T>* GetComponentList();

			/**
			*@brief
			*/
			template <typename T>
			T* AddComponent(const Enjon::EntityHandle& entity);

			/**
			*@brief
			*/
			Component* AddComponent( const MetaClass* compCls, const Enjon::EntityHandle& handle );

			/**
			*@brief
			*/
			template <typename T>
			void RemoveComponent(Entity* entity);

			/**
			*@brief
			*/
			template <typename T>
			T* GetComponent(Entity* entity);

			/**
			*@brief
			*/
			void Destroy(const EntityHandle& entity);

			/**
			*@brief
			*/
			const Vector<Entity*>& GetActiveEntities() const
			{ 
				return mActiveEntities; 
			}

		protected:

			/**
			* @brief
			*/
			Component* GetComponent( const EntityHandle& entity, const u32& ComponentID );

		private:
			/**
			*@brief
			*/
			void EntityManager::Cleanup();

			/**
			*@brief
			*/
			static b8 CompareEntityIDs(const Entity* a, const Entity* b);

			/**
			*@brief
			*/
			template <typename T>
			void DetachComponentFromEntity(Entity* entity);

			/**
			*@brief
			*/
			u32 FindNextAvailableID();

			/**
			*@brief
			*/
			void UpdateAllActiveTransforms(f32 dt);

		private:
			EntityStorage 				mEntities;
			ComponentBaseArray 			mComponents;	
			ActiveEntityList 			mActiveEntities;
			MarkedForDestructionList 	mMarkedForDestruction;
			u32 						mNextAvailableID = 0;
	};

	#include "Entity/Entity.inl"
	#include "Entity/EntityManager.inl"
}

#endif


