#include "BehaviorTree/BehaviorTreeManager.h"
#include "TestTree.h"

using namespace BT;

namespace BTManager
{
	std::unordered_map<std::string, BehaviorTree*> Trees;

	void Init()
	{
		// Trees are added from the Editor
		// Not sure how I feel about that...
		// Maybe I should load them in from here instead...
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

	void DebugPrintTrees()
	{
		std::cout << "Trees: " << std::endl;

		for (auto it = Trees.begin(); it != Trees.end(); ++it)
		{
			std::cout << "\t" << it->first << std::endl;
		}
	}

}