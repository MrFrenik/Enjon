#ifndef ENJON_CONDITIONAL_H
#define ENJON_CONDITIONAL_H

#include "BehaviorNode.h"
#include "ECS/Transform3DSystem.h"

namespace BT
{
	class ConditionalTask : public Task<ConditionalTask>
	{
		public:

			ConditionalTask(BehaviorTree* BT, bool (*A)(BehaviorTree* BT)) 
			{ 
				BTree = BT; 
				Action = A;
				Init(); 
			}
			~ConditionalTask() {}

			void Init() { State = BehaviorNodeState::INVALID; }

			std::string String()
			{
				return std::string("ConditionalTask");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				// auto SO = BTree->GetBlackBoard()->GetComponent<StateObject*>("States");
				// auto SS = &SO->GetData()->States;

				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

				if (State != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					State = BehaviorNodeState::RUNNING;
				}

				// Run the action
				bool B = Action(BTree);

				if (B) 	{ State = BehaviorNodeState::SUCCESS; SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS; return BehaviorNodeState::SUCCESS; } 
				else	{ State = BehaviorNodeState::FAILURE; SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE; return BehaviorNodeState::FAILURE; }
			}

		private:
			bool (*Action)(BehaviorTree* BT);

	};	

	class IsTargetWithinRangeConditional : public Task<IsTargetWithinRangeConditional>
	{
		public:
			IsTargetWithinRangeConditional(BehaviorTree* BT) 
			{ 
				BTree = BT; 
				Action = [](BT::BehaviorTree* BT)
					   {
					   		auto P = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition");
					   		auto Manager = ECS::Systems::EntitySystem::World();
					   		auto ID = BT->GetBlackBoard()->GetComponent<Enjon::uint32>("EID");
					   		
					   		auto Target = P->GetData();
					   		auto ai = ID->GetData();

					   		auto AI = &Manager->TransformSystem->Transforms[ai].Position.XY();
					   		auto V = &Manager->TransformSystem->Transforms[ai].Velocity.XY();
					   		auto H = Manager->AttributeSystem->HealthComponents[ai].Health; 

					   		auto Distance = AI->DistanceTo(Target.XY());

					   		if (Distance <= 100.0f) 
					   		{
					   			return true;
					   		}

					   		return false;
					   };

				Init(); 
			}
			~IsTargetWithinRangeConditional() {}

			void Init() { State = BehaviorNodeState::INVALID; }

			std::string String()
			{
				return std::string("IsTargetWithinRangeConditional");
			}

			BehaviorNodeState Run()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;
				SO->CurrentNode = this;

				if (State != BehaviorNodeState::RUNNING)
				{
					SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
					State = BehaviorNodeState::RUNNING;
				}

				// Run the action
				bool B = Action(BTree);

				if (B) 	{ State = BehaviorNodeState::SUCCESS; SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS; return BehaviorNodeState::SUCCESS; } 
				else	{ State = BehaviorNodeState::FAILURE; SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE; return BehaviorNodeState::FAILURE; }
			}

		private:
			bool (*Action)(BehaviorTree* BT);

	};	

}


#endif