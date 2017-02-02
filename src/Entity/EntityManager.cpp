#include "Entity/EntityManager.h"
#include "Entity/Component.h"

#include <array>
#include <vector>
#include <assert.h>

#include <stdio.h>

namespace Enjon { 

	EntityManager::EntityManager()
	{
		for (auto i = 0; i < Components.size(); i++)
		{
			Components.at(i) = nullptr;
		}

		NextAvailableID = 0;
		mEntities = new EntityHandle[MAX_ENTITIES];
	}

	EntityManager::~EntityManager()
	{
	}

	EntityHandle* EntityManager::Allocate()
	{
		assert(NextAvailableID < MAX_ENTITIES);
		EntityHandle Entity(this);
		auto id = Entity.GetID();
		mEntities[id] = Entity;
		return &mEntities[id];
	}
}