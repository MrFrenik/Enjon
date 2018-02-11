/*
#include "Graphics/SceneNode.h"
#include "Entity/EntityManager.h"

#include <assert.h>
#include <algorithm>

namespace Enjon
{
	//-----------------------------------------
	SceneNode::SceneNode()
		: mParent(nullptr), mChildren()
	{
	}

	//-----------------------------------------
	SceneNode::~SceneNode()
	{
	}

	//-----------------------------------------
	SceneNode& SceneNode::AddChild(SceneNode* child)
	{
		// Set parent to this
		child->SetParent(this);

		// Make sure child doesn't exist in vector before pushing back
		auto query = std::find(mChildren.begin(), mChildren.end(), child);
		if (query == mChildren.end())
		{
			mChildren.push_back(child);
		}
		else
		{
			// Log a warning mesage here
		}

		return *this;
	}

	//-----------------------------------------
	void SceneNode::DetachChild(SceneNode* child)
	{
		// Make sure child exists
		assert(child != nullptr);

		// Find and erase
		mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), child), mChildren.end());

		// Remove parent from child
		child->RemoveParent();
	}

	//-----------------------------------------
	void SceneNode::SetParent(SceneNode* parent)
	{
		// Make sure this child doesn't have a parent
		assert(parent != nullptr);
		assert(mParent == nullptr);

		// Set parent
		mParent = parent;
	}

	//-----------------------------------------
	SceneNode* SceneNode::RemoveParent()
	{
		// Asset parent exists
		assert(mParent != nullptr);

		// Capture pointer
		SceneNode* retNode = mParent;

		// Remove parent
		mParent = nullptr;

		return retNode;
	}
	
}
*/