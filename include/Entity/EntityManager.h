#ifndef ENJON_ENTITY_MANAGER_H
#define ENJON_ENTITY_MANAGER_H
#pragma once

#define MAX_ENTITIES 500000

#include <Entity/Component.h>
#include <TypeCatalog.h>

#include <array>
#include <vector>
#include <cstdint>
#include <assert.h>

using eid32 = uint32_t;

struct ComponentWrapperBase
{
	virtual void Base() = 0;
};

template <typename T>
struct ComponentWrapper : public ComponentWrapperBase
{
	void Base() override {}
	std::vector<T> Components;
	std::array<uint32_t, MAX_ENTITIES> ComponentIndexMap;
};

struct EntityManager;
struct EntityHandle;

void DetachAll(EntityManager* Manager, EntityHandle* Handle);

struct EntityHandle
{
	eid32 ID;	
	TypeCatalog::ComponentBitset ComponentMask;
	EntityManager* Manager;

	EntityHandle()
	{
		// This needs to be set to some null variable
		ID = 0;
		ComponentMask = TypeCatalog::ComponentBitset(0);
		Manager = nullptr;	
	}

	EntityHandle(EntityManager* _Manager)
	{
		// This needs to be set to some null variable
		ID = 0;
		ComponentMask = TypeCatalog::ComponentBitset(0);
		Manager = _Manager;	
	}

	// TODO(John): Take care of removing all attached components here
	~EntityHandle()
	{
		// Clean up all components
		// I'm not happy about this, but for now just have to iterate through all types
		// and check to see if the entity has that particular one
		DetachAll(Manager, this);
	}

	template <typename T>
	bool HasComponent()
	{
		return ((ComponentMask & TypeCatalog::GetBitMask<T>()) != 0);
	}

	template <typename T>
	T* GetComponent()
	{
		// Assert that it has component
		assert(HasComponent<T>());

		// Assert entity manager exists
		assert(Manager != nullptr);

		// Get component wrapper
		auto CWrapper = static_cast<ComponentWrapper<T>*>(Manager->Components.at(static_cast<size_t>(TypeCatalog::GetType<T>())));

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
};



struct EntityManager
{
	EntityManager()
	{
		for (auto i = 0; i < Components.size(); i++)
		{
			Components.at(i) = nullptr;
		}

		NextAvailableID = 0;
	}

	EntityHandle* CreateEntity()
	{
		assert(NextAvailableID < MAX_ENTITIES);
		EntityHandle Entity(this);
		Entity.ID = NextAvailableID++;
		Entities.at(Entity.ID) = Entity;
		return &Entities.at(Entity.ID);
	}

	template <typename T>
	void RegisterComponent()
	{
		auto index = static_cast<size_t>(TypeCatalog::GetType<T>());
		assert(Components.at(index) == nullptr);
		auto C = new ComponentWrapper<T>;	
		Components.at(index) = C;
	}

	template <typename T>
	std::vector<T>* GetComponentList()
	{
		auto index = static_cast<size_t>(TypeCatalog::GetType<T>());
		assert(Components.at(index) != nullptr);
		return &(static_cast<ComponentWrapper<T>*>(Components.at(index))->Components);	
	}

	template <typename T>
	T* Attach(EntityHandle* Entity)
	{
		// Check to make sure isn't already attached to this entity
		assert(!Entity->HasComponent<T>());

		// Get index into vector and assert that entity manager has this component
		auto index = static_cast<size_t>(TypeCatalog::GetType<T>());
		assert(Components.at(index) != nullptr);

		auto CWrapper = static_cast<ComponentWrapper<T>*>(Components.at(index));
		auto ComponentList = &(CWrapper->Components);

		// Create new component and push back into components list
		// and set entity as its owner
		T NewComponent;
		NewComponent.Entity = Entity;
		ComponentList->push_back(NewComponent);

		// Store component index for entity in component index map
		CWrapper->ComponentIndexMap.at(Entity->ID) = ComponentList->size() - 1;

		// Set bitmask field for component
		Entity->ComponentMask |= TypeCatalog::GetBitMask<T>();

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
		Entity->ComponentMask ^= TypeCatalog::GetBitMask<T>();
	}

	// NOTE(John): Should only ever be called in context of detaching a component from an entity!
	template <typename T>
	void RemoveComponent(EntityHandle* Entity)
	{
		// Get index into vector and assert that entity manager has this component
		auto index = static_cast<size_t>(TypeCatalog::GetType<T>());
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
		auto CWrapper = static_cast<ComponentWrapper<T>*>(Components.at(TypeCatalog::GetType<T>()));

		// Get component index
		auto ComponentIndex = CWrapper->ComponentIndexMap.at(Entity->ID);

		// Return component
		return &(CWrapper->Components.at(ComponentIndex));
	}

	void DetachAll(EntityHandle* Entity);

	std::array<EntityHandle, MAX_ENTITIES> Entities;
	std::array<ComponentWrapperBase*, static_cast<size_t>(TypeCatalog::ComponentType::COUNT)> Components;	
	uint32_t NextAvailableID;
};

inline void DetachAll(EntityManager* Manager, EntityHandle* Handle)
{
	Manager->DetachAll(Handle);	
}


#endif


/*

	template <typename T>
	struct ComponentReference
	{
		std::vector<T>* ComponentList;		
	}





*/