#ifndef REPEATER_H
#define REPEATER_H

#include "BehaviorNode.h"

namespace BT
{
	class Repeater : public Decorator<Repeater>
	{
		public:

			Repeater() {}
			Repeater(BlackBoard* BB, i32 C = 1) : Count(C) { this->BB = BB; State = BehaviorNodeState::INVALID; Child = nullptr; }
			Repeater(BlackBoard* BB, BehaviorNodeBase* B, i32 C = 1) : Count(C) { this->BB = BB; State = BehaviorNodeState::INVALID; Child = B; }
			Repeater(BehaviorTree* BT, BehaviorNodeBase* B, i32 C = 1) : Count(C) { this->BTree = BT; State = BehaviorNodeState::INVALID; Child = B; }
			Repeater(BehaviorTree* BT, i32 C = 1) : Count(C) { this->BTree = BT; State = BehaviorNodeState::INVALID; Child = nullptr; }
			~Repeater() {}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				if (SS->at(this->TreeIndex) != BehaviorNodeState::RUNNING)
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
				// BehaviorNodeState S = Child->GetState();
				BehaviorNodeState S = SS->at(Child->GetIndex());

				if (S == BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				else
				{
					// Inifinte loop, so do not decrement count
					if (Count < 0) 
					{
						SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
						return BehaviorNodeState::RUNNING;
					}

					else
					{
						Count--;
						if (Count <= 0)
						{
							std::cout << "Repeater succeeded." << std::endl;
							State = BehaviorNodeState::SUCCESS;
							SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
							return BehaviorNodeState::SUCCESS;
						}
						else
						{
							State = BehaviorNodeState::RUNNING;
							std::cout << "Running again..." << std::endl;
							SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
							return BehaviorNodeState::RUNNING;
						}
					}
				}
			}

		private:
			i32 Count;
	};

	// This is a lazy class
	// When monitoring is implemented, this will go away
	class RepeaterWithBBRead : public Decorator<RepeaterWithBBRead>
	{
		public:

			// SimpleTask(BehaviorTree* BT, void (*Action)(BehaviorTree*))
			// {
			// 	this->BTree = BT;
			// 	this->Action = Action;
			// 	State = BehaviorNodeState::INVALID;
			// }

			RepeaterWithBBRead(BehaviorTree* BT, i32 (*A)(BehaviorTree*), BehaviorNodeBase* B = nullptr, i32 C = 1) 
				: Count(C)
			{
				this->Child = B;
				this->BTree = BT;
				Action = A;
				State = BehaviorNodeState::INVALID; 
			}
		
			~RepeaterWithBBRead() {}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				if (SS->at(this->TreeIndex) != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
				}

				if (Child == nullptr) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE;
					return BehaviorNodeState::FAILURE;
				}

				Count--;

				// Process child
				Child->Run();

				// Get child's state after running
				// BehaviorNodeState S = Child->GetState();
				BehaviorNodeState S = SS->at(Child->GetIndex());

				if (S == BehaviorNodeState::RUNNING) 
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				else
				{
					// Need to read count from BB, so call action
					Count = Action(BTree);

					// Inifinte loop, so do not decrement count
					if (Count < 0) 
					{
						SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
						return BehaviorNodeState::RUNNING;
					}

					else
					{
						if (Count <= 0)
						{
							State = BehaviorNodeState::SUCCESS;
							SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
							return BehaviorNodeState::SUCCESS;
						}
						else
						{
							State = BehaviorNodeState::RUNNING;
							SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
							return BehaviorNodeState::RUNNING;
						}
					}
				}
			}

		private:
			i32 Count;
			i32 (*Action)(BehaviorTree*);
	};

}

#endif




















