#ifndef ENJON_INVERTER_H
#define ENJON_INVERTER_H

#include "BehaviorNode.h"

namespace BT
{
	class Inverter : public Decorator<Inverter>
	{
		public:

			Inverter(BehaviorTree* BT, BehaviorNodeBase* B = nullptr) { this->BTree = BT; Init(); Child = B; Type = BehaviorNodeType::DECORATOR; }
			~Inverter() {}

			void Init() { State = BehaviorNodeState::INVALID; Child = nullptr; }

			std::string String()
			{
				return std::string("Inverter");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

				if (State != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					State = BehaviorNodeState::RUNNING;
				}	


				if (Child == nullptr) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;	
					return BehaviorNodeState::FAILURE;
				}

				// Process child
				Child->Run();

				// Get child's state after running
				BehaviorNodeState S = SS->at(Child->GetIndex());

				switch(S)
				{
					case BehaviorNodeState::FAILURE: 
					{
						State = SUCCESS;
						SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	
						return BehaviorNodeState::SUCCESS;
						break;
					}

					case BehaviorNodeState::SUCCESS:
					{
						State = FAILURE;
						SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;	
						return BehaviorNodeState::FAILURE;
						break;
					}

					default: 
					{
						SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
						return BehaviorNodeState::SUCCESS; 
						break;
					}
				}
			}
	};	
}


#endif