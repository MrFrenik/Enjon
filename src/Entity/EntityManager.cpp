#include "Entity/EntityManager.h"
#include "Entity/Component.h"

#include <array>
#include <vector>
#include <assert.h>

#include <stdio.h>

namespace Enjon {

	//---------------------------------------------------------------
	Entity::Entity()
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mComponentMask(Enjon::ComponentBitset(0)),
	  mManager(nullptr),
	  mWorldTransformDirty(true)
	{
	}

	//---------------------------------------------------------------
	Entity::Entity(EntityManager* manager)
	: mID(MAX_ENTITIES), 
	  mState(EntityState::INACTIVE), 
	  mComponentMask(Enjon::ComponentBitset(0)),
	  mManager(manager),
	  mWorldTransformDirty(true)
	{
	}

	// TODO(John): Take care of removing all attached components here
	//---------------------------------------------------------------
	Entity::~Entity()
	{
		mManager->Destroy(this);		
	}

	//---------------------------------------------------------------
	void Entity::Reset()
	{
		mID = MAX_ENTITIES;
		mState = EntityState::INACTIVE;
		mWorldTransformDirty = true;
		mComponentMask = Enjon::ComponentBitset(0);
		mManager = nullptr;
		mComponents.clear();
	}

	//---------------------------------------------------------------
	void Entity::SetID(u32 id)
	{
		mID = id;
	}

	//---------------------------------------------------------------
	void Entity::SetLocalTransform(EM::Transform& transform)
	{
		mLocalTransform = transform;
		mWorldTransformDirty = true;
	}

	//---------------------------------------------------------------
	EM::Transform Entity::GetLocalTransform()
	{
		return mLocalTransform;
	}

	//---------------------------------------------------------------
	EM::Transform Entity::GetWorldTransform()
	{
		// If dirty, then calcualte world transform
		if (mWorldTransformDirty)
		{
			CalculateWorldTransform();
		}

		// Return world transform
		return mWorldTransform;
	}

	//---------------------------------------------------------------
	void Entity::CalculateWorldTransform()
	{
		if (mParent == nullptr) 
		{
			mWorldTransform = mLocalTransform;
			mWorldTransformDirty = false;
			return;
		}

		// Start with local transform. If no parent exists, then we return this.
		EM::Transform result = mLocalTransform;

		// Iterate through all parents and multiply local transform with parents
		for (Entity* p = mParent; p != nullptr; p = p->GetParent())
		{
			result *= p->GetLocalTransform();
		}

		// Cache it off for now
		mWorldTransform = result;

		mWorldTransformDirty = false;
	}

	//---------------------------------------------------------------
	void Entity::SetPosition(EM::Vec3& position)
	{
		mLocalTransform.SetPosition(position);
		mWorldTransformDirty = true;

		// If has children, propogate transform
		if (HasChildren())
		{
			SetAllChildWorldTransformsDirty();
		}
	}

	//---------------------------------------------------------------
	void Entity::SetScale(EM::Vec3& scale)
	{
		mLocalTransform.SetScale(scale);
		mWorldTransformDirty = true;

		// If has children, propogate transform
		if (HasChildren())
		{
			SetAllChildWorldTransformsDirty();
		}
	}

	//---------------------------------------------------------------
	void Entity::SetRotation(EM::Quaternion& rotation)
	{
		mLocalTransform.SetRotation(rotation);
		mWorldTransformDirty = true;

		// If has children, propogate transform
		if (HasChildren())
		{
			SetAllChildWorldTransformsDirty();
		}
	}

	//---------------------------------------------------------------
	EM::Vec3 Entity::GetLocalPosition()
	{
		return mLocalTransform.GetPosition();
	}

	//---------------------------------------------------------------
	EM::Vec3 Entity::GetLocalScale()
	{
		return mLocalTransform.GetScale();
	}

	//---------------------------------------------------------------
	EM::Quaternion Entity::GetLocalRotation()
	{
		return mLocalTransform.GetRotation();
	}

	//---------------------------------------------------------------
	EM::Vec3 Entity::GetWorldPosition()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetPosition();
	}

	//---------------------------------------------------------------
	EM::Vec3 Entity::GetWorldScale()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetScale();
	}

	//---------------------------------------------------------------
	EM::Quaternion Entity::GetWorldRotation()
	{
		if (mWorldTransformDirty) CalculateWorldTransform();
		return mWorldTransform.GetRotation();
	}

	//-----------------------------------------
	Entity* Entity::AddChild(Entity* child)
	{
		// Set parent to this
		child->SetParent(this);

		// Make sure child doesn't exist in vector before pushing back
		auto query = std::find(mChildren.begin(), mChildren.end(), child);
		if (query == mChildren.end())
		{
			// Add child to children list
			mChildren.push_back(child);

			// Calculate its world transform with respect to parent
			child->CalculateWorldTransform();
		}
		else
		{
			// Log a warning mesage here
		}

		return this;
	}

	//-----------------------------------------
	void Entity::DetachChild(Entity* child)
	{
		// Make sure child exists
		assert(child != nullptr);

		// Find and erase
		mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), child), mChildren.end());

		// Recalculate world transform of child
		child->CalculateWorldTransform();

		// Set parent to nullptr
		child->mParent = nullptr;
	}

	//-----------------------------------------
	void Entity::SetParent(Entity* parent)
	{
		// Make sure this child doesn't have a parent
		assert(parent != nullptr);
		assert(mParent == nullptr);

		// Set parent to this
		mParent = parent;

		// Calculate world transform
		CalculateWorldTransform();
	}

	//-----------------------------------------
	Entity* Entity::RemoveParent()
	{
		// Asset parent exists
		assert(mParent != nullptr);

		// Remove child from parent
		mParent->DetachChild(this);

		// Capture pointer
		Entity* retNode = mParent;

		// Set parent to nullptr
		mParent = nullptr;

		return retNode;
	}

	//---------------------------------------------------------------
	b8 Entity::HasChildren()
	{
		return (mChildren.size() > 0);
	}

	//---------------------------------------------------------------
	b8 Entity::HasParent()
	{
		return (mParent != nullptr);
	}

	//---------------------------------------------------------------
	void Entity::SetAllChildWorldTransformsDirty()
	{
		for (auto& c : mChildren)
		{
			// Set dirty to true
			c->mWorldTransformDirty = true;

			// Iterate through child's children to set their state dirty as well
			c->SetAllChildWorldTransformsDirty();
		}
	}

	//---------------------------------------------------------------
	void Entity::UpdateAllChildTransforms()
	{
		// Maybe this should just be to set their flags to dirty?
		for (auto& c : mChildren)
		{
			c->mWorldTransformDirty = true;
			c->CalculateWorldTransform();
		}
	}

	//---------------------------------------------------------------
	void Entity::UpdateComponentTransforms()
	{

	}

	//---------------------------------------------------------------
	EntityManager::EntityManager()
	{
		for (auto i = 0; i < mComponents.size(); i++)
		{
			mComponents.at(i) = nullptr;
		}

		mNextAvailableID = 0;
		mEntities = new std::array<Entity, MAX_ENTITIES>;
	}

	//---------------------------------------------------------------
	EntityManager::~EntityManager()
	{
		// Detach all components from entities
		for (u32 i = 0; i < MAX_ENTITIES; ++i)
		{
			Destroy(&mEntities->at(i));	
		}
		delete[] mEntities;

		// Deallocate all components
		for (u32 i = 0; i < mComponents.size(); ++i)
		{
			delete mComponents.at(i);
			mComponents.at(i) = nullptr;
		}
	}

	//---------------------------------------------------------------
	u32 EntityManager::FindNextAvailableID()
	{
		// Iterate from current available id to MAX_ENTITIES
		for (u32 i = mNextAvailableID; i < MAX_ENTITIES; ++i)
		{
			if (mEntities->at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Iterate from 0 to mNextAvailableID
		for (u32 i = 0; i < mNextAvailableID; ++i)
		{
			if (mEntities->at(i).mState == EntityState::INACTIVE)
			{
				mNextAvailableID = i;
				return mNextAvailableID;
			}
		}

		// Other wise return MAX_ENTITIES, since there are no entity slots left
		return MAX_ENTITIES;
	}

	//---------------------------------------------------------------
	Entity* EntityManager::Allocate()
	{
		// Grab next available id and assert that it's valid
		u32 id = FindNextAvailableID();
		assert(id < MAX_ENTITIES);

		// Find entity in array and set values
		Entity* entity = &mEntities->at(id);
		entity->mID = id;				
		entity->mState = EntityState::ACTIVE; 
		entity->mManager = this;

		// Push back live entity into active entity vector
		mActiveEntities.push_back(entity);

		// Return entity handle
		return entity;
	}

	//---------------------------------------------------------------
	void EntityManager::Destroy(Entity* entity)
	{
		// Assert that entity isn't already null
		assert(entity != nullptr);

		// Push for deferred removal from active entities
		mMarkedForDestruction.push_back(entity);

		// Iterate through entity component list and detach
		for (auto& c : entity->mComponents)
		{
			// Detach(c);
			c->Destroy();
		}	

		// Set state to inactive
		entity->mState = EntityState::INACTIVE;

		// Set bitmask to 0
		entity->Reset();
	}

	//--------------------------------------------------------------
	void EntityManager::Cleanup()
	{
		// Move through dirty list and remove from active entities
		for (auto& c : mMarkedForDestruction)
		{
			// Remove from active entities
			mActiveEntities.erase(std::remove(mActiveEntities.begin(), mActiveEntities.end(), c), mActiveEntities.end());
		}

		mMarkedForDestruction.clear();
	}

	//--------------------------------------------------------------
	void EntityManager::Update(f32 dt)
	{
		// Clean any entities that were marked for destruction
		Cleanup();
	} 
}


















