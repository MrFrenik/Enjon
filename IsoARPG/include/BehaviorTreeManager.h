#ifndef BEHAVIOR_TREE_MANAGER_H
#define BEHAVIOR_TREE_MANAGER_H

#include "BehaviorTree/BehaviorTree.h"

#include <unordered_map>

namespace BTManager
{
	void Init();

	void AddBehaviorTree(std::unordered_map<std::string, BT::BehaviorTree*>& M, std::string N, BT::BehaviorTree* T);

	BT::BehaviorTree* GetBehaviorTree(std::string N);
}

#endif