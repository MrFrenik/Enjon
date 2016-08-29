#ifndef ENJON_BBWRITE_H
#define ENJON_BBWRITE_H

#include "BehaviorNode.h"

namespace BT
{
	template <typename T>
	class BBWrite : public Decorator<BBWrite<T>>
	{
		public:

			BBWrite(BehaviorTree* BT, std::string ComponentName, T Value, BehaviorNodeBase* B = nullptr)
			{ 
				BTree = BT; 
				ComponentKey = ComponentName;
				ComponentValue = Value;
				Action = [](BT::BehaviorTree* BT, BBWrite* BBW)
							{
						   		BT->GetBlackBoard()->GetComponent<T>(BBW->ComponentKey)->SetData(BBW->ComponentValue);
							};
				Init(); 
				Child = B; 
			}

			~BBWrite(){}

			void Init()
			{
				State = BehaviorNodeState::INVALID;	
			}

			std::string String()
			{
				return std::string("BBWrite");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
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
					Action(BTree, this);
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
			void (*Action)(BehaviorTree* BT, BBWrite* BBW);
			std::string ComponentKey;
			T ComponentValue;



	};	
}


#endif