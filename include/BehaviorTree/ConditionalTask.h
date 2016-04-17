#ifndef CONDITIONAL_H
#define CONDITIONAL_H

#include "BehaviorNode.h"

namespace BT
{
	class ConditionalTask : public Task<ConditionalTask>
	{
		public:

			ConditionalTask(BehaviorTree* BT, bool (*A)(BehaviorTree* BT)) 
			{ 
				BTree = BT; 
				Action = A;
				Init(); 
			}
			~ConditionalTask() {}

			void Init() { State = BehaviorNodeState::INVALID; }

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				if (State != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					State = BehaviorNodeState::RUNNING;
				}

				// Run the action
				bool B = Action(BTree);

				if (B) 	{ State = BehaviorNodeState::SUCCESS; SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS; return BehaviorNodeState::SUCCESS; } 
				else	{ State = BehaviorNodeState::FAILURE; SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE; return BehaviorNodeState::FAILURE; }
			}

		private:
			bool (*Action)(BehaviorTree* BT);

	};	
}


#endif