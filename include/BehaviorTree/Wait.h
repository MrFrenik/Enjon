#ifndef ENJON_WAIT_H
#define ENJON_WAIT_H

#include "BehaviorTree/BehaviorNode.h"
#include "BehaviorTree/Task.h"
#include "Math/Vec3.h"

// struct Timer
// {
	// Unioned with Vec3

// 	float CurrentTime;   	-- x
// 	float DT;  				-- y
// 	float Time;				-- z
// };

namespace BT
{
	class Wait : public Task<Wait>
	{
		public:

			Wait(BehaviorTree* BT, EM::Vec3 T, Enjon::i32 L = 0)
			{
				this->BTree = BT;
				Clock = T;
				TotalLoops = L;
				Init();
			}

			~Wait(){}

			void Init()
			{
				Clock.x = 0.0f;
				CurrentLoop = 0;
				State = BehaviorNodeState::INVALID;
				Type = BehaviorNodeType::LEAF;

			}

			std::string String()
			{
				return std::string("Wait");
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
					Reset();
				}

				Clock.x += Clock.y;
				if (Clock.x >= Clock.z)
				{
					if (TotalLoops > CurrentLoop)
					{
						Enjon::u32 D = TotalLoops - CurrentLoop;
						CurrentLoop++;
						Clock.x = 0.0f;
					}
					else
					{
						SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
						State = BehaviorNodeState::SUCCESS;
						return BehaviorNodeState::SUCCESS;
					}
				}

				else
				{
					float D = Clock.z - Clock.x; 
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
				return BehaviorNodeState::RUNNING;
			}

			inline void Reset()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;

				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;

				State = BehaviorNodeState::RUNNING;
				Clock.x = 0.0f;
				CurrentLoop = 0;
			}

		private:
			EM::Vec3 Clock;
			Enjon::i32 TotalLoops;
			Enjon::i32 CurrentLoop;
	};

	class WaitWBBRead : public Task<WaitWBBRead>
	{
		public:

			WaitWBBRead(BehaviorTree* BT)
			{
				this->BTree = BT;
				Init();
			}

			~WaitWBBRead(){}

			void Init()
			{
				State = BehaviorNodeState::INVALID;
				Type = BehaviorNodeType::LEAF;
			}

			std::string String()
			{
				return std::string("WaitWBBRead");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				auto Clock = &BTree->GetBlackBoard()->GetComponent<EM::Vec3>("Timer")->GetData();
				SO->CurrentNode = this;

				if (SS->at(this->TreeIndex) != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					Reset();
				}

				Clock->x += Clock->y;
				if (Clock->x >= Clock->z)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
					State = BehaviorNodeState::SUCCESS;
					return BehaviorNodeState::SUCCESS;
				}

				else
				{
					float D = Clock->z - Clock->x; 
					BTree->GetBlackBoard()->GetComponent<EM::Vec3>("Timer")->SetData(*Clock);
					// std::cout << "Time Left: " << D << std::endl;
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
				return BehaviorNodeState::RUNNING;
			}

			inline void Reset()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				auto Clock = &BTree->GetBlackBoard()->GetComponent<EM::Vec3>("Timer")->GetData();

				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;

				State = BehaviorNodeState::RUNNING;
				Clock->x = 0.0f;
				BTree->GetBlackBoard()->GetComponent<EM::Vec3>("Timer")->SetData(*Clock);
			}

		private:
	};
}

#endif