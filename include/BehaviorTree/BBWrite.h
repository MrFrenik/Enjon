#ifndef BBWRITE_H
#define BBWRITE_H

#include "BehaviorNode.h"

namespace BT
{
	class BBWrite : public Decorator<BBWrite>
	{
		public:

			BBWrite(BehaviorTree* BT, void (*A)(BehaviorTree* BT), BehaviorNodeBase* B = nullptr){ BTree = BT; Action = A; Init(); Child = B; }
			~BBWrite(){}

			void Init()
			{
				State = BehaviorNodeState::INVALID;	
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = BTree->GetBlackBoard()->GetComponent<StateObject*>("States");
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
					Action(BTree);
					return BehaviorNodeState::SUCCESS;
				}
				if (S == BehaviorNodeState::FAILURE)
				{
					State = BehaviorNodeState::FAILURE;
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				return BehaviorNodeState::RUNNING;
			}

		private:
			void (*Action)(BehaviorTree* BT);



	};	
}


#endif