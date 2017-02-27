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

	enum class EntityTransformSpace
	{
		LOCAL,
		WORLD
	};

	class EntityHandle
	{
		friend EntityManager; 
		public:
			EntityHandle();
			EntityHandle(EntityManager* manager);
			~EntityHandle();

			u32 GetID() { return mID; }

			template <typename T>
			bool HasComponent();

			template <typename T>
			T* GetComponent();

			template <typename T>
			T* Attach();

			template <typename T>
			void Detach();

			void SetLocalTransform(EM::Transform& transform);
			EM::Transform GetLocalTransform();

			void SetWorldTransform(EM::Transform& transform);
			EM::Transform GetWorldTransform();

			void SetWorldPosition(EM::Vec3& position);
			void SetWorldScale(EM::Vec3& scale);
			void SetWorldOrientation(EM::Quaternion& orientation);

			void SetLocalPosition(EM::Vec3& position);
			void SetLocalScale(EM::Vec3& scale);
			void SetLocalOrientation(EM::Quaternion& orientation);

			EM::Vec3 GetLocalPosition();
			EM::Vec3 GetLocalScale();
			EM::Quaternion GetLocalOrientation();

			EM::Vec3 GetWorldPosition();
			EM::Vec3 GetWorldScale();
			EM::Quaternion GetWorldOrientation();

			EntityHandle* GetParent() { return mParent; }

			EM::Transform mLocalTransform;
			EM::Transform mWorldTransform;

			EntityHandle* AddChild(EntityHandle* child);
			void DetachChild(EntityHandle* child);	

		protected:
			void SetParent(EntityHandle* parent);
			EntityHandle* RemoveParent();

		private:
			void SetID(u32 id);
			void Reset();

			void UpdateAllChildrenTransforms();
			void UpdateComponentTransforms();

			u32 mID;	
			EntityHandle* mParent;
			Enjon::ComponentBitset mComponentMask;
			Enjon::EntityManager* mManager;
			std::vector<Component*> mComponents;
			std::vector<EntityHandle*> mChildren;
			Enjon::EntityState mState;
	};

	class EntityManager
	{
		friend EntityHandle;
		public:
			EntityManager();
			~EntityManager();

			EntityHandle* Allocate();

			void Update(f32 dt);

			template <typename T>
			void RegisterComponent();

			template <typename T>
			std::vector<T>* GetComponentList();

			template <typename T>
			T* Attach(Enjon::EntityHandle* entity);

			template <typename T>
			void Detach(EntityHandle* entity);

			template <typename T>
			T* GetComponent(EntityHandle* entity);

			void Destroy(EntityHandle* entity);

			const std::vector<EntityHandle*>& GetActiveEntities() { return mActiveEntities; }

		private:
			void EntityManager::Cleanup();

			template <typename T>
			void RemoveComponent(EntityHandle* entity);

			u32 FindNextAvailableID();

			std::array<EntityHandle, MAX_ENTITIES>* mEntities;
			std::array<ComponentWrapperBase*, static_cast<u32>(MAX_COMPONENTS)> mComponents;	
			std::vector<EntityHandle*> mActiveEntities;
			std::vector<EntityHandle*> mMarkedForDestruction;
			u32 mNextAvailableID = 0;
	};

	#include "Entity/EntityHandle.inl"
	#include "Entity/EntityManager.inl"
}

#endif



