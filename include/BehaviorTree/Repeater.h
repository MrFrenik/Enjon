#ifndef ENJON_REPEATER_H
#define ENJON_REPEATER_H

#include "BehaviorNode.h"

namespace BT
{
	class Repeater : public Decorator<Repeater>
	{
		public:

			Repeater() {}
			Repeater(BlackBoard* BB, Enjon::i32 C = 1) : Count(C) { this->BB = BB; State = BehaviorNodeState::INVALID; Child = nullptr; Type = BehaviorNodeType::DECORATOR; }
			Repeater(BlackBoard* BB, BehaviorNodeBase* B, Enjon::i32 C = 1) : Count(C) { this->BB = BB; State = BehaviorNodeState::INVALID; Child = B; Type = BehaviorNodeType::DECORATOR;}
			Repeater(BehaviorTree* BT, BehaviorNodeBase* B, Enjon::i32 C = 1) : Count(C) { this->BTree = BT; State = BehaviorNodeState::INVALID; Child = B; Type = BehaviorNodeType::DECORATOR;}
			Repeater(BehaviorTree* BT, Enjon::i32 C = 1) : Count(C) { this->BTree = BT; State = BehaviorNodeState::INVALID; Child = nullptr; Type = BehaviorNodeType::DECORATOR; }
			~Repeater() {}

			std::string String()
			{
				return std::string("Repeater");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

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
			Enjon::i32 Count;
	};

	class RepeatUntilFail : public Decorator<RepeatUntilFail>
	{
		public:

			RepeatUntilFail() {}
			RepeatUntilFail(BlackBoard* BB, Enjon::i32 C = 1) : Count(C) { this->BB = BB; State = BehaviorNodeState::INVALID; Child = nullptr; }
			RepeatUntilFail(BlackBoard* BB, BehaviorNodeBase* B, Enjon::i32 C = 1) : Count(C) { this->BB = BB; State = BehaviorNodeState::INVALID; Child = B; }
			RepeatUntilFail(BehaviorTree* BT, BehaviorNodeBase* B, Enjon::i32 C = 1) : Count(C) { this->BTree = BT; State = BehaviorNodeState::INVALID; Child = B; }
			RepeatUntilFail(BehaviorTree* BT, Enjon::i32 C = 1) : Count(C) { this->BTree = BT; State = BehaviorNodeState::INVALID; Child = nullptr; }
			~RepeatUntilFail() {}

			std::string String()
			{
				return std::string("RepeatUntilFail");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

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
				BehaviorNodeState S = SS->at(Child->GetIndex());

				if (S == BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				// Success, so repeat
				else if (S == BehaviorNodeState::SUCCESS)
				{
					State = BehaviorNodeState::RUNNING;
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				// Failure, so leave repeat
				else 
				{
					State = BehaviorNodeState::SUCCESS;
					SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
					return BehaviorNodeState::SUCCESS;
				}
			}

		private:
			Enjon::i32 Count;
	};

	class RepeatForever : public Decorator<RepeatForever>
	{
		public:

			RepeatForever() {}
			RepeatForever(BehaviorTree* BT, Enjon::i32 C = 1)
			{ 
				this->BTree = BT; 
				State = BehaviorNodeState::INVALID; 
				Child = nullptr; 
			}
			~RepeatForever() {}

			std::string String()
			{
				return std::string("RepeatForever");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

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
				BehaviorNodeState S = SS->at(Child->GetIndex());

				if (S == BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				// Success, so repeat
				else
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
					return BehaviorNodeState::SUCCESS;
				}
			}
	};

	// This is a lazy class
	// When monitoring is implemented, this will go away
	class RepeaterWithBBRead : public Decorator<RepeaterWithBBRead>
	{
		public:

			RepeaterWithBBRead(BehaviorTree* BT, Enjon::i32 (*A)(BehaviorTree*), BehaviorNodeBase* B = nullptr, Enjon::i32 C = 1) 
				: Count(C)
			{
				this->Child = B;
				this->BTree = BT;
				Action = A;
				State = BehaviorNodeState::INVALID; 
			}
		
			~RepeaterWithBBRead() {}

			std::string String()
			{
				return std::string("RepeaterWithBBRead");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

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
						if (Count == 0)
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
			Enjon::i32 Count;
			Enjon::i32 (*Action)(BehaviorTree*);
	};


}

#endif




















