#ifndef ENJON_CONDITIONAL_DECORATOR_H
#define ENJON_CONDITIONAL_DECORATOR_H

#include "BehaviorNode.h"

namespace BT
{
	class ConditionalDecorator : public Decorator<ConditionalDecorator>
	{
		public:
			ConditionalDecorator(BlackBoard* BB, bool (*A)(BlackBoard* BB))
			{
				Action = A;
				this->BB = BB;	
				Init();
			}

			ConditionalDecorator(BehaviorTree* BT, bool (*A)(BlackBoard* BB))
			{
				Action = A;
				this->BTree = BT;	
				Init();
			}

			void Init()
			{
				State = BehaviorNodeState::INVALID;
				Child = nullptr;
			}

			std::string String()
			{
				return std::string("ConditionalDecorator");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				// auto SO = BTree->GetBlackBoard()->GetComponent<StateObject*>("States");
				// auto SS = &SO->GetData()->States;

				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

				if (Child == nullptr) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				if (State != BehaviorNodeState::RUNNING)
				{
					Reset();
				}

				// Run action before processing child
				bool B = Action(BB);

				if (!B)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					State = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				// If success, then process child
				auto ReturnState = Child->Run();

				SS->at(this->TreeIndex) = SS->at(Child->GetIndex());

				// Set state to child's state
				State = ReturnState;

				return ReturnState;
			}

			inline void Reset()
			{
				// Get State Object from BlackBoard
				// auto SO = BTree->GetBlackBoard()->GetComponent<StateObject*>("States");
				// auto SS = &SO->GetData()->States;

				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				
				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;			
				State = BehaviorNodeState::RUNNING;
			}


		private:	
			bool (*Action)(BlackBoard* BB);

	};	
}


#endif