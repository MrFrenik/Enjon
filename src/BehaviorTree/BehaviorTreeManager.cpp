#include "BehaviorTree/BehaviorTreeManager.h"
#include "TestTree.h"

using namespace BT;

namespace BTManager
{
	std::unordered_map<std::string, BehaviorTree*> Trees;

	void Init()
	{
		// AddBehaviorTree(Trees, "TestTree", TestTree());	
	}

	/* Add spritesheets to the spritesheetmanager */
	void AddBehaviorTree(std::unordered_map<std::string, BehaviorTree*>& M, std::string N, BehaviorTree* T)
	{
		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		M[N] = T;
	}

	void AddBehaviorTree(std::string N, BehaviorTree* T)
	{
		Trees[N] = T;
	}

	/* Get specificed animation with given name */
	BehaviorTree* GetBehaviorTree(std::string N)
	{
		auto it = Trees.find(N);
		if (it != Trees.end()) return it->second;
		else
		{
			printf("Behavior Tree Not found!");
			return nullptr;
		}
	}

}