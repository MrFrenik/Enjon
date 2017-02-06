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

	class EntityHandle
	{
		friend EntityManager; 
		public:
			EntityHandle()
			{
				// This needs to be set to some null variable
				mID = 0;
				mComponentMask = Enjon::ComponentBitset(0);
				mManager = nullptr;	
			}

			EntityHandle(EntityManager* _Manager)
			{
				// This needs to be set to some null variable
				mID = 0;
				mComponentMask = Enjon::ComponentBitset(0);
				mManager = _Manager;	
			}

			// TODO(John): Take care of removing all attached components here
			~EntityHandle()
			{
			}

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
				assert(Manager != nullptr);

				// Get component wrapper
				auto CWrapper = static_cast<ComponentWrapper<T>*>(mManager->Components.at(static_cast<size_t>(TypeCatalog::GetType<T>())));

				// Get component index
				auto ComponentIndex = CWrapper->ComponentIndexMap.at(this->ID);

				// Return component
				return &(CWrapper->Components.at(ComponentIndex));
			}

			template <typename T>
			T* Attach()
			{
				// Check to make sure isn't already attached to this entity
				assert(!HasComponent<T>());

				// Make sure entity manager isn't null
				assert(Manager != nullptr);

				// Get index into vector and assert that entity manager has this component
				auto index = static_cast<size_t>(TypeCatalog::GetType<T>());
				assert(Manager->Components.at(index) != nullptr);

				auto CWrapper = static_cast<ComponentWrapper<T>*>(Manager->Components.at(index));
				auto ComponentList = &(CWrapper->Components);

				// Create new component and push back into components list
				// and set entity as its owner
				T NewComponent;
				NewComponent.Entity = this;
				ComponentList->push_back(NewComponent);

				// Store component index for entity in component index map
				CWrapper->ComponentIndexMap.at(this->ID) = ComponentList->size() - 1;

				// Set bitmask field for component
				this->ComponentMask |= TypeCatalog::GetBitMask<T>();

				// Return newly created component to allow for chaining
				return &(CWrapper->Components.back());	
			}

			template <typename T>
			void Detach()
			{
				// Check to make sure isn't already attached to this entity
				assert(HasComponent<T>());

				// Make sure entity manager isn't null
				assert(Manager != nullptr);

				// Remove component from entity manager components
				Manager->RemoveComponent<T>(this);

				// Set bitmask field for component
				this->ComponentMask ^= TypeCatalog::GetBitMask<T>();
			}

			void SetPosition(EM::Vec3& position);
			void SetScale(EM::Vec3& scale);
			void SetOrientation(EM::Quaternion& orientation);

			EM::Vec3& GetPosition() { return mTransform.GetPosition(); }
			EM::Vec3& GetScale() { return mTransform.GetScale(); }
			EM::Quaternion& GetOrientation() { return mTransform.GetOrientation(); }

		private:
			u32 mID;	
			Enjon::ComponentBitset mComponentMask;
			Enjon::EntityManager* mManager;
			Enjon::Math::Transform mTransform;
			std::vector<Component*> mComponents;
	};

	class EntityManager
	{
		public:
			EntityManager();
			~EntityManager();

			EntityHandle* Allocate();

			template <typename T>
			void RegisterComponent()
			{
				static_assert(std::is_base_of<Component, T>::value, 
					"EntityManager::RegisterComponent:: T must inherit from Component.");
				auto index = static_cast<size_t>(Enjon::GetComponentType<T>());
				assert(Components.at(index) == nullptr);
				auto C = new ComponentWrapper<T>;	
				Components.at(index) = C;
			}

			template <typename T>
			std::vector<T>* GetComponentList()
			{
				auto index = static_cast<size_t>(Enjon::GetComponentType<T>());
				assert(Components.at(index) != nullptr);
				return &(static_cast<ComponentWrapper<T>*>(Components.at(index))->Components);	
			}

			template <typename T>
			T* Attach(Enjon::EntityHandle* Entity)
			{
				// Check to make sure isn't already attached to this entity
				assert(!Entity->HasComponent<T>());

				// Get index into vector and assert that entity manager has this component
				auto index = static_cast<size_t>(Enjon::GetComponentType<T>());
				assert(Components.at(index) != nullptr);

				auto CWrapper = static_cast<ComponentWrapper<T>*>(Components.at(index));
				auto ComponentList = &(CWrapper->Components);

				// Create new component and push back into components list
				// and set entity as its owner
				T NewComponent;
				NewComponent.SetEntity(Entity);
				ComponentList->push_back(NewComponent);

				// Store component index for entity in component index map
				CWrapper->ComponentIndexMap.at(Entity->GetID()) = ComponentList->size() - 1;

				// Set bitmask field for component
				Entity->mComponentMask |= Enjon::GetComponentBitMask<T>();

				// Return newly created component to allow for chaining
				return &(CWrapper->Components.back());	
			}

			template <typename T>
			void Detach(EntityHandle* Entity)
			{
				// Check to make sure isn't already attached to this entity
				assert(Entity->HasComponent<T>());

				// Remove component from entity manager components
				RemoveComponent<T>(Entity);

				// Set bitmask field for component
				Entity->mComponentMask ^= Enjon::GetComponentBitMask<T>();
			}

			// NOTE(John): Should only ever be called in context of detaching a component from an entity!
			template <typename T>
			void RemoveComponent(EntityHandle* Entity)
			{
				// Get index into vector and assert that entity manager has this component
				auto index = static_cast<size_t>(Enjon::GetComponentType<T>());
				assert(Components.at(index) != nullptr);

				auto CWrapper = static_cast<ComponentWrapper<T>*>(Components.at(index));
				auto ComponentList = &(CWrapper->Components);
				auto ComponentIndex = CWrapper->ComponentIndexMap.at(Entity->ID);
				auto ComponentIndexMap = &CWrapper->ComponentIndexMap;

				// If an the last element
				if (ComponentList->size() == 1 || ComponentIndex == ComponentList->size() - 1)
				{
					// Just pop back
					ComponentList->pop_back();
				}

				// Otherwise need to perform swap
				else
				{
					// Swap the back with index at question of components
					ComponentList->at(ComponentIndex) = ComponentList->back();
					ComponentList->pop_back();

					auto Component = &ComponentList->at(ComponentIndex);

					// Now to fix the newly moved component in the ComponentIndexMap
					ComponentIndexMap->at(ComponentList->at(ComponentIndex).Entity->ID) = ComponentIndex;
				}
			}

			template <typename T>
			T* GetComponent(EntityHandle* Entity)
			{
				// Assert that it has component
				assert(Entity->HasComponent<T>());

				// Assert entity manager exists
				assert(Entity != nullptr);

				// Get component wrapper
				auto CWrapper = static_cast<ComponentWrapper<T>*>(Components.at(Enjon::GetComponentType<T>()));

				// Get component index
				auto ComponentIndex = CWrapper->ComponentIndexMap.at(Entity->ID);

				// Return component
				return &(CWrapper->Components.at(ComponentIndex));
			}

		private:
			EntityHandle* mEntities;
			std::array<ComponentWrapperBase*, static_cast<size_t>(MAX_COMPONENTS)> Components;	
			u32 NextAvailableID;
	};

}

#endif


