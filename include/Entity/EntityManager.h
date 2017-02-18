#ifndef ENJON_ENTITY_MANAGER_H
#define ENJON_ENTITY_MANAGER_H
#pragma once

#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityDefines.h"
#include "Math/Transform.h"

#include "System/Types.h"

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

			void SetPosition(EM::Vec3& position);
			void SetScale(EM::Vec3& scale);
			void SetOrientation(EM::Quaternion& orientation);
			void SetParent(EntityHandle* parent);

			EntityHandle* GetParent() const { return mParent; }

			EM::Vec3 GetPosition() { return mTransform.GetPosition(); }
			EM::Vec3 GetScale() { return mTransform.GetScale(); }
			EM::Quaternion GetOrientation() { return mTransform.GetOrientation(); }

		private:
			void SetID(u32 id);
			void Reset();

			void UpdateComponentTransforms();

			u32 mID;	
			EntityHandle* mParent;
			Enjon::ComponentBitset mComponentMask;
			Enjon::EntityManager* mManager;
			Enjon::Math::Transform mTransform;
			std::vector<Component*> mComponents;
			Enjon::EntityState mState;
	};

	class EntityManager
	{
		friend EntityHandle;
		public:
			EntityManager();
			~EntityManager();

			EntityHandle* Allocate();

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

		private:

			template <typename T>
			void RemoveComponent(EntityHandle* entity);

			u32 FindNextAvailableID();

			std::array<EntityHandle, MAX_ENTITIES>* mEntities;
			std::array<ComponentWrapperBase*, static_cast<u32>(MAX_COMPONENTS)> mComponents;	
			u32 mNextAvailableID = 0;
	};

	#include "Entity/EntityHandle.inl"
	#include "Entity/EntityManager.inl"
}

#endif



