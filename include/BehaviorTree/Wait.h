#ifndef WAIT_H
#define WAIT_H

#include "BehaviorNode.h"

typedef struct 
{
	float CurrentTime;
	float DT;
	float Time;	
} Timer;

namespace BT
{
	class Wait : public Task<Wait>
	{
		public:

			Wait(BlackBoard* BB, Timer T, i32 L = 0)
			{
				this->BB = BB;
				Clock = T;
				TotalLoops = L;
				Init();
			}

			~Wait(){}

			void Init()
			{
				Clock.CurrentTime = 0.0f;
				CurrentLoop = 0;
				State = BehaviorNodeState::INVALID;

			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BB->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				if (SS->at(this->TreeIndex) != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					Reset();
				}

				Clock.CurrentTime += Clock.DT;
				if (Clock.CurrentTime >= Clock.Time)
				{
					if (TotalLoops > CurrentLoop)
					{
						u32 D = TotalLoops - CurrentLoop;
						CurrentLoop++;
						Clock.CurrentTime = 0.0f;
						std::cout << D << " more loop(s) to go." << std::endl;
					}
					else
					{
						SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS;
						State = BehaviorNodeState::SUCCESS;
						std::cout << "Done Waiting!" << std::endl;
						return BehaviorNodeState::SUCCESS;
					}
				}

				else
				{
					float D = Clock.Time - Clock.CurrentTime; 
					std::cout << D << " more to go." << std::endl;
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					return BehaviorNodeState::RUNNING;
				}

				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
				return BehaviorNodeState::RUNNING;
			}

			inline void Reset()
			{
				State = BehaviorNodeState::RUNNING;
				Clock.CurrentTime = 0.0f;
				CurrentLoop = 0;
			}

		private:
			Timer Clock;
			i32 TotalLoops;
			i32 CurrentLoop;
	};
}

#endif