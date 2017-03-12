#ifndef ENJON_ENTITY_MANAGER_H
#define ENJON_ENTITY_MANAGER_H
#pragma once

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityDefines.h"
#include "Math/Transform.h"

#include "System/Types.h"
#include "Defines.h"

#include <array>
#include <vector>
#include <cstdint>
#include <assert.h>

namespace Enjon {

	class EntityManager;

	enum class EntityState
	{
		INACTIVE,
		ACTIVE
	};

	class Entity
	{
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

			/// @brief Gets component from entity, if exists
			template <typename T>
			T* GetComponent();

			/// @brief Attaches component to entity, if exists
			template <typename T>
			T* Attach();

			/// @brief Detaches component from entity, if exists
			template <typename T>
			void Detach();

			/// @brief Sets local transform of entity
			void SetLocalTransform(EM::Transform& transform);

			/// @brief Gets local transform of entity relative to parent entity, if exists
			EM::Transform GetLocalTransform();

			/// @brief Gets World transform of entity which calculates world transform if dirty flag is set
			EM::Transform GetWorldTransform();

			/// @brief Sets local position of entity relative to parent, if exists
			void SetPosition(EM::Vec3& position);

			/// @brief Sets local scale of entity relative to parent, if exists
			void SetScale(EM::Vec3& scale);

			/// @brief Sets local orientation of entity relative to parent, if exists
			void SetRotation(EM::Quaternion& rotation);

			/// @brief Gets local position of entity relative to parent, if exists
			EM::Vec3 GetLocalPosition();

			/// @brief Gets local scale of entity relative to parent, if exists
			EM::Vec3 GetLocalScale();

			/// @brief Gets local rotation of entity relative to parent, if exists
			EM::Quaternion GetLocalRotation();

			/// @brief Gets World position of entity which calculates world transform if dirty flag is set
			EM::Vec3 GetWorldPosition();

			/// @brief Gets World scale of entity which calculates world transform if dirty flag is set
			EM::Vec3 GetWorldScale();

			/// @brief Gets World rotation of entity which calculates world transform if dirty flag is set
			EM::Quaternion GetWorldRotation();

			/// @brief Gets parent of this entity, returns nullptr if doesn't exist
			Entity* GetParent() { return mParent; }

			/// @brief Registers a child with this entity
			Entity* AddChild(Entity* child);

			/// @brief Removes child from entity, if exists
			void DetachChild(Entity* child);	

			/// @brief Sets parent of entity, if one doesn't already exist
			void SetParent(Entity* parent);

			/// @brief Removes parent from entity, if one exists
			Entity* RemoveParent();

			/// @brief Returns whether or not has parent
			b8 Entity::HasParent();

			/// @brief Returns whether or not has children
			b8 Entity::HasChildren();

		protected:
			/// @brief Calculates world transform with respect to parent heirarchy
			void CalculateWorldTransform();


		private:
			/// @brief Sets id of entity - Entity Manager is responsible for this
			void SetID(u32 id);

			/// @brief Resets all appropriate fields and member variables for entity
			void Reset();

			/// @brief Sets all child states to be updated
			void SetAllChildWorldTransformsDirty();

			/// @brief Propogates transform down through all children
			void UpdateAllChildTransforms();

			/// @brief Propogates transform down through all components
			void UpdateComponentTransforms();

		private:
			u32 mID;	
			b32 mWorldTransformDirty; 					// NOTE(): Necessary struct padding for alignment. Not too happy about it.
			Entity* mParent;
			EM::Transform mLocalTransform;
			EM::Transform mWorldTransform;
			Enjon::ComponentBitset mComponentMask;
			Enjon::EntityManager* mManager;
			std::vector<Component*> mComponents;
			std::vector<Entity*> mChildren;
			Enjon::EntityState mState;
	};

	class EntityManager
	{
		friend Entity;
		public:
			EntityManager();
			~EntityManager();

			Entity* Allocate();

			void Update(f32 dt);

			template <typename T>
			void RegisterComponent();

			template <typename T>
			std::vector<T>* GetComponentList();

			template <typename T>
			T* Attach(Enjon::Entity* entity);

			template <typename T>
			void Detach(Entity* entity);

			template <typename T>
			T* GetComponent(Entity* entity);

			void Destroy(Entity* entity);

			const std::vector<Entity*>& GetActiveEntities() { return mActiveEntities; }

		private:
			void EntityManager::Cleanup();

			template <typename T>
			void RemoveComponent(Entity* entity);

			u32 FindNextAvailableID();

			std::array<Entity, MAX_ENTITIES>* mEntities;
			std::array<ComponentWrapperBase*, static_cast<u32>(MAX_COMPONENTS)> mComponents;	
			std::vector<Entity*> mActiveEntities;
			std::vector<Entity*> mMarkedForDestruction;
			u32 mNextAvailableID = 0;
	};

	#include "Entity/Entity.inl"
	#include "Entity/EntityManager.inl"
}

#endif



