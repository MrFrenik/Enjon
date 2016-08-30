#ifndef ENJON_CONDITIONAL_H
#define ENJON_CONDITIONAL_H

#include "BehaviorNode.h"
#include "ECS/Transform3DSystem.h"
#include "System/Internals.h"

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
				Type = BehaviorNodeType::LEAF;
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

	class IsTargetWithinRange : public Task<IsTargetWithinRange>
	{
		public:
			IsTargetWithinRange(BehaviorTree* BT) 
			{ 
				BTree = BT; 
				Action = [](BT::BehaviorTree* BT, Enjon::Internals::EFloat& D)
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

					   		if (Distance <= D.Value) 
					   		{
					   			return true;
					   		}

					   		return false;
					   };

				Type = BehaviorNodeType::LEAF;
				Init(); 
			}
			~IsTargetWithinRange() {}

			void Init() { State = BehaviorNodeState::INVALID; }

			std::string String()
			{
				return std::string("IsTargetWithinRange");
			}

			void AddChild(BehaviorNodeBase* B)
			{}

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
				bool B = Action(BTree, Distance);

				if (B) 	{ State = BehaviorNodeState::SUCCESS; SS->at(this->TreeIndex) = BehaviorNodeState::SUCCESS; return BehaviorNodeState::SUCCESS; } 
				else	{ State = BehaviorNodeState::FAILURE; SS->at(this->TreeIndex) = BehaviorNodeState::FAILURE; return BehaviorNodeState::FAILURE; }
			}

		public:
			Enjon::Internals::EFloat Distance;

		private:
			bool (*Action)(BehaviorTree*, Enjon::Internals::EFloat&);

	};	

}


#endif