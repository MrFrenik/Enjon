#ifndef SELECTOR_H
#define SELECTOR_H

#include "BehaviorNode.h"

namespace BT
{
	class Selector : public Composite<Selector>
	{
		public:

			Selector(BlackBoard* BB, std::initializer_list<BehaviorNodeBase*> L = {})
				: Composite(L)
			{ 
				this->BB = BB; Init(); 
			}

			~Selector(){}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BB->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				if (Children.size() == 0) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				// Will fail if there are no children
				if (SS->at(this->TreeIndex) != BehaviorNodeState::RUNNING) { Reset(); }

				// Get first child
				auto CN = Children.begin();
				auto Current = *CN;

				// Return if null
				if (Current == nullptr) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				// Find child that is ready to be processed
				auto CState = SS->at(Current->GetIndex());
				while (CState == BehaviorNodeState::SUCCESS || 
					   CState == BehaviorNodeState::FAILURE)
				{
					// Increment iterator
					CN++;

					if (CN >= Children.end()) 
					{
						State = BehaviorNodeState::FAILURE;
						std::cout << "Selector Failed!" << std::endl;
						SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
						return BehaviorNodeState::FAILURE;
					}
					else
					{
						Current = *CN;
						CState = SS->at(Current->GetIndex());
					}
				}

				// Run node
				Current->Run();

				// Get child's state
				auto S = CurrentNode->GetState();

				if (S == BehaviorNodeState::SUCCESS)
				{
					State = BehaviorNodeState::SUCCESS;
					std::cout << "Selector succeeded!" << std::endl;
					SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
					return BehaviorNodeState::SUCCESS;
				}

				// if (S == BehaviorNodeState::FAILURE)
				// {
				// 	Itr++;

				// 	// Succeed when all children succeed
				// 	if (Itr >= Children.end())
				// 	{
				// 		State = BehaviorNodeState::FAILURE;
				// 		std::cout << "Selector failed!" << std::endl;
				// 		return BehaviorNodeState::FAILURE;
				// 	}

				// 	else CurrentNode = *Itr;

				// }

				return BehaviorNodeState::RUNNING;
			}
	};
}



#endif