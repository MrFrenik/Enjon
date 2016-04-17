#ifndef CONDITIONAL_H
#define CONDITIONAL_H

#include "BehaviorNode.h"

namespace BT
{
	class ConditionalTask : public Task<ConditionalTask>
	{
		public:

			ConditionalTask(BlackBoard* bb, bool (*A)(BlackBoard* BB)) 
			{ 
				BB = bb; 
				Action = A;
				Init(); 
			}
			~ConditionalTask() {}

			void Init() { State = BehaviorNodeState::INVALID; }

			BehaviorNodeState Run()
			{
				if (State != BehaviorNodeState::RUNNING)
				{
					State = BehaviorNodeState::RUNNING;
				}

				// Run the action
				bool B = Action(BB);

				if (B) 	{ State = BehaviorNodeState::SUCCESS; return BehaviorNodeState::SUCCESS; } 
				else	{ State = BehaviorNodeState::FAILURE; return BehaviorNodeState::FAILURE; }
			}

		private:
			bool (*Action)(BlackBoard* BB);

	};	
}


#endif