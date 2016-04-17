 #include "BehaviorTree/BehaviorTree.h"

#include <iostream>

// namespace BT
// {
// 	StateObject* BehaviorTree::CreateStateObject()
// 	{
// 		StateObject* SO = new StateObject();

// 		// Return SO if no root
// 		if (Root == nullptr) return SO;

// 		// Get total child size of tree by recursively calling get child size + Root
// 		auto S = Root->GetChildSize() + 1;

// 		std::cout << "Size: " << S << std::endl;

// 		// Push back states into SO
// 		for (u32 i = 0; i < S; i++)
// 		{
// 			SO->States.push_back(BehaviorNodeState::INVALID);
// 		}

// 		return SO;
// 	}

// 	void BehaviorTree::Run(StateObject* SO)
// 	{
// 		// HotSwap in State Object (SO) to BlackBoard to be operated on
// 		auto S = static_cast<BlackBoardComponent<StateObject*>*>(BB->GetComponent("States"));
// 		S->SetData(SO);

// 		// Now run tree on SO
// 		Root->Run();

// 		// Set Tree state for now 
// 		State = S->GetData()->States.at(Root->GetIndex());
// 	}

// 	BlackBoard* BehaviorTree::CreateBlackBoard()
// 	{
// 		// Make SO
// 		auto SO = CreateStateObject();	

// 		BlackBoard* BB = new BlackBoard();

// 		// Put in BB
// 		BB->AddComponent("States", new BlackBoardComponent<StateObject*>(SO));

// 		return BB;
// 	}

// 	void BehaviorTree::SetTreeIndicies()
// 	{
// 		u32 i = 0;
// 		std::cout << "Root: " << i << std::endl;
// 		auto R = Root->SetIndicies(i);
// 	}
// }

