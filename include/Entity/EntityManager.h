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
			EntityHandle(EntityManager* _Manager);
			~EntityHandle();

			u32 GetID() { return mID; }

			template <typename T>
			bool HasComponent()
			{
				return ((mComponentMask & Enjon::GetComponentBitMask<T>()) != 0);
			}

			template <typename T>
			T* GetComponent()
			{
				// Assert that it has component
				assert(HasComponent<T>());

				// Assert entity manager exists
				assert(mManager != nullptr);

				return mManager->GetComponent<T>(this);
			}

			template <typename T>
			T* Attach()
			{
				// Check to make sure isn't already attached to this entity
				assert(!HasComponent<T>());

				// Make sure that entity manager isn't null
				assert(mManager != nullptr);

				return mManager->Attach<T>(this);
			}

			template <typename T>
			void Detach()
			{
				// Check to make sure isn't already attached to this entity
				assert(HasComponent<T>());

				// Make sure entity manager isn't null
				assert(mManager != nullptr);

				// Detach component
				mManager->Detach<T>(this);
			}

			void SetPosition(EM::Vec3& position);
			void SetScale(EM::Vec3& scale);
			void SetOrientation(EM::Quaternion& orientation);
			void SetParent(EntityHandle* parent);

			EntityHandle* GetParent() const { return mParent; }

			EM::Vec3& GetPosition() { return mTransform.GetPosition(); }
			EM::Vec3& GetScale() { return mTransform.GetScale(); }
			EM::Quaternion& GetOrientation() { return mTransform.GetOrientation(); }

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
			void RegisterComponent()
			{
				static_assert(std::is_base_of<Component, T>::value, "EntityManager::RegisterComponent:: T must inherit from Component.");
				u32 index = static_cast<u32>(Enjon::GetComponentType<T>());
				assert(mComponents.at(index) == nullptr);
				ComponentWrapper<T>* C = new ComponentWrapper<T>;	
				mComponents.at(index) = C;
			}

			template <typename T>
			std::vector<T>* GetComponentList()
			{
				u32 index = Enjon::GetComponentType<T>();
				assert(Components.at(index) != nullptr);
				return &(static_cast<ComponentWrapper<T>*>(Components.at(index))->mComponentPtrs);	
			}

			template <typename T>
			T* Attach(Enjon::EntityHandle* entity)
			{
				// Assert entity is valid
				assert(entity != nullptr);
				// Check to make sure isn't already attached to this entity
				assert(!entity->HasComponent<T>());

				// Entity id
				u32 eid = entity->GetID();

				// Get index into vector and assert that entity manager has this component
				u32 index = Enjon::GetComponentType<T>();
				assert(mComponents.at(index) != nullptr);

				ComponentWrapper<T>* cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(index));
				auto cMap = &(cWrapper->mComponentMap);
				auto componentList = &(cWrapper->mComponentPtrs);

				// Create new component and place into map
				auto query = cMap->find(eid);
				if (query == cMap->end())
				{
					T component;
					component.SetEntity(entity);
					component.SetID(index);
					component.SetBase(cWrapper); 
					component.mEntityID = entity->mID;
					cWrapper->mComponentMap[eid] = component;
					componentList->push_back(&cWrapper->mComponentMap[eid]);

					// Set bitmask field for component
					entity->mComponentMask |= Enjon::GetComponentBitMask<T>();

					// Get component ptr and push back into entity components
					auto compPtr = &cWrapper->mComponentMap[eid];
					entity->mComponents.push_back(compPtr);

					return compPtr;
				}

				// Otherwise the entity already has the component
				assert(false);
			}

			template <typename T>
			void Detach(EntityHandle* entity)
			{
				// Check to make sure isn't already attached to this entity
				assert(entity->HasComponent<T>());

				// Remove component from entity manager components
				RemoveComponent<T>(entity);

				// Set bitmask field for component
				entity->mComponentMask ^= Enjon::GetComponentBitMask<T>();
			}

			template <typename T>
			T* GetComponent(EntityHandle* entity)
			{
				// Assert that it has component
				assert(entity->HasComponent<T>());

				// Assert entity manager exists
				assert(entity != nullptr);

				u32 eid = entity->GetID();

				// Get component wrapper
				auto cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(Enjon::GetComponentType<T>()));

				// Return component
				auto* cMap = &cWrapper->mComponentMap;
				auto query = cMap->find(eid);
				if (query != cMap->end())
				{
					return &query->second;
				}
			}

			void Destroy(EntityHandle* entity);

		private:

			template <typename T>
			void RemoveComponent(EntityHandle* entity)
			{
				u32 idx = Enjon::GetComponentType<T>();
				assert(mComponents.at(idx) != nullptr);

				auto cWrapper = static_cast<ComponentWrapper<T>*>(mComponents.at(idx));
				auto cPtrList = &cWrapper->mComponentPtrs;
				auto cMap = &cWrapper->mComponentMap;

				auto compPtr = &cWrapper->mComponentMap[entity->mID];

				// Search through entity component list and remove reference
				auto entComps = &entity->mComponents;
				entComps->erase(std::remove(entComps->begin(), entComps->end(), compPtr), entComps->end());

				// Remove ptr from point list map
				cPtrList->erase(std::remove(cPtrList->begin(), cPtrList->end(), compPtr), cPtrList->end());	

				// Finally remove from map
				cMap->erase(entity->mID);
			}

			u32 FindNextAvailableID();

			// EntityHandle* mEntities;
			std::array<EntityHandle, MAX_ENTITIES>* mEntities;
			std::array<ComponentWrapperBase*, static_cast<u32>(MAX_COMPONENTS)> mComponents;	
			u32 mNextAvailableID = 0;
	};

}

#endif


