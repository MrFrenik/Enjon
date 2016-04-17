#ifndef INVERTER_H
#define INVERTER_H

#include "BehaviorNode.h"

namespace BT
{
	class Inverter : public Decorator<Inverter>
	{
		public:

			Inverter(BlackBoard* BB) { this->BB = BB; Init(); }
			Inverter(BlackBoard* BB, BehaviorNodeBase* B) { this->BB = BB; Init(); Child = B; }
			~Inverter() {}

			void Init() { State = BehaviorNodeState::INVALID; Child = nullptr; }

			BehaviorNodeState Run()
			{

				if (State != BehaviorNodeState::RUNNING)
				{
					State = BehaviorNodeState::RUNNING;
				}	

				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BB->GetComponent("States"));
				auto SS = &SO->GetData()->States;

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
						std::cout << "Inverted Failure to Success." << std::endl;
						SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	
						return BehaviorNodeState::SUCCESS;
						break;
					}

					case BehaviorNodeState::SUCCESS:
					{
						State = FAILURE;
						std::cout << "Inverted Success to Failure." << std::endl;
						SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;	
						return BehaviorNodeState::FAILURE;
						break;
					}

					default: return BehaviorNodeState::SUCCESS; break;
				}
			}
	};	
}


#endif