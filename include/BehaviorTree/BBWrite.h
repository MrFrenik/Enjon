#ifndef BBWRITE_H
#define BBWRITE_H

#include "BehaviorNode.h"

namespace BT
{
	class BBWrite : public Decorator<BBWrite>
	{
		public:

			BBWrite(BlackBoard* bb, void (*A)(BlackBoard* BB)){ BB = bb; Action = A; Init(); }
			BBWrite(BehaviorTree* BT, void (*A)(BlackBoard* BB)){ BTree = BT; Action = A; Init(); }
			~BBWrite(){}

			void Init()
			{
				Child = nullptr;
				State = BehaviorNodeState::INVALID;	
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				if (Child == nullptr) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				if (State != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					State = BehaviorNodeState::RUNNING;
				}

				// Run child
				Child->Run();

				// Get state of child after running
				BehaviorNodeState S = SS->at(Child->GetIndex());

				// If success, then call write function
				if (S == BehaviorNodeState::SUCCESS)
				{
					State = BehaviorNodeState::SUCCESS;
					SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
					Action(BB);
					return BehaviorNodeState::SUCCESS;
				}
				if (S == BehaviorNodeState::FAILURE)
				{
					State = BehaviorNodeState::FAILURE;
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}
			}

		private:
			void (*Action)(BlackBoard* BB);



	};	
}


#endif