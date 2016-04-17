#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include "BehaviorNode.h"

namespace BT
{
	// A wrapper for our nodes
	// class BehaviorTree
	// {
	// 	public:

	// 		BehaviorTree(BehaviorNodeBase* R, BlackBoard* bb) : Root(R), BB(bb) 
	// 		{
	// 			// Set up BB with empty state object
	// 			StateObject* SO = CreateStateObject();

	// 			// Put in BB
	// 			BB->AddComponent("States", new BlackBoardComponent<StateObject*>(SO));

	// 			State = BehaviorNodeState::RUNNING;
	// 		}

	// 		BehaviorTree() : Root(nullptr), BB(new BlackBoard()) 
	// 		{
	// 			// Set up BB with empty state object
	// 			StateObject* SO = CreateStateObject();

	// 			// Put in BB
	// 			BB->AddComponent("States", new BlackBoardComponent<StateObject*>(SO));

	// 			State = BehaviorNodeState::RUNNING;
	// 		}

	// 		void End() { SetTreeIndicies(); }

	// 		~BehaviorTree(){}

	// 		StateObject* CreateStateObject();
	// 		BlackBoard* CreateBlackBoard(); 
	// 		void Run(StateObject* SO);

	// 		template <typename T>
	// 		inline T* GetRoot() const { return static_cast<T*>(Root); }
	// 		inline void SetRoot(BehaviorNodeBase* B) { Root = B; }
	// 		inline BlackBoard* GetBlackBoard() { return BB; }
	// 		inline BehaviorNodeState GetState() const { return State; }


	// 	private:
	// 		BehaviorNodeBase* Root;
	// 		BlackBoard* BB;
	// 		BehaviorNodeState State;

	// 	private:
	// 		void SetTreeIndicies();
	// };
}

#endif