#include "Entity/EntityManager.h"
#include "Entity/Component.h"

#include <array>
#include <vector>
#include <assert.h>

#include <stdio.h>

namespace Enjon {

	void EntityHandle::SetID(u32 id)
	{
		mID = id;
	}

	void EntityHandle::SetPosition(EM::Vec3& position)
	{
		mTransform.SetPosition(position);	
	}

	void EntityHandle::SetScale(EM::Vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void EntityHandle::SetOrientation(EM::Quaternion& orientation)
	{
		mTransform.SetOrientation(orientation);
	}

	EntityManager::EntityManager()
	{
		for (auto i = 0; i < mComponents.size(); i++)
		{
			mComponents.at(i) = nullptr;
		}

		mNextAvailableID = 0;
		mEntities = new std::array<EntityHandle, MAX_ENTITIES>;
	}

	EntityManager::~EntityManager()
	{
		// Detach all components from entities
		// Deallocate all components
		// Deallocate all entities
	}

	EntityHandle* EntityManager::Allocate()
	{
		assert(mNextAvailableID < MAX_ENTITIES);
		u32 id = mNextAvailableID++;
		EntityHandle* entity = &mEntities->at(id);
		entity->SetID(id);				
		entity->mManager = this;
		return entity;
	}
}