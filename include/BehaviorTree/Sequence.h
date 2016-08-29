#ifndef ENJON_SEQUENCE_H
#define ENJON_SEQUENCE_H

#include "BehaviorNode.h"


namespace BT
{
	class Sequence : public Composite<Sequence>
	{
		public:

			Sequence(BlackBoard* BB, std::initializer_list<BehaviorNodeBase*> L = {}) : Composite(L)
			{ 
				this->BB = BB; Init(); 
				Type = BehaviorNodeType::COMPOSITE;
			}

			Sequence(BehaviorTree* BT, std::initializer_list<BehaviorNodeBase*> L = {}) : Composite(L)
			{ 
				this->BTree = BT; Init(); 
				Type = BehaviorNodeType::COMPOSITE;
			}

			~Sequence(){}

			std::string String()
			{
				return std::string("Sequence");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

				// Will fail if there are no children
				if (Children.size() == 0)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

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
					CN++;

					if (CN >= Children.end()) 
					{
						State = BehaviorNodeState::SUCCESS;
						SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
						return BehaviorNodeState::SUCCESS;
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
				auto S = SS->at(Current->GetIndex());

				// Fail if there's a failure
				if (S == BehaviorNodeState::FAILURE)
				{
					State = BehaviorNodeState::FAILURE;
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				return BehaviorNodeState::RUNNING;
			}
	};
}





#endif