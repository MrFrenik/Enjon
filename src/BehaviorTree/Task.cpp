#ifndef ENJON_TASK_H
#define ENJON_TASK_H

// #include "BehaviorTree/Task.h"
// #include "ECS/ComponentSystems.h"
// #include "ECS/Transform3DSystem.h"
// #include "ECS/AttributeSystem.h"

namespace BT
{
	/*
	template <typename T>
	class Task : public BehaviorNode<Task<T>>
	{
		public:

			void Init()
			{
				static_cast<T*>(this)->Init();
			}

			BehaviorNodeState Run()
			{
				return static_cast<T*>(this)->Run();
			}

			void Reset()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;

				// Reset state
				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
			}

			inline u32 GetChildSize() { return 0; }

			inline u32 SetIndicies(u32 I) { this->TreeIndex = I; return I; }

			inline std::vector<BehaviorNodeBase*> GetAllChildren() { std::vector<BehaviorNodeBase*> C; return C; }
	};


	class SimpleTask : public Task<SimpleTask>
	{
		public:

		SimpleTask(BehaviorTree* BT, void (*Action)(BehaviorTree*))
		{
			this->BTree = BT;
			this->Action = Action;
			State = BehaviorNodeState::INVALID;
		}

		BehaviorNodeState Run()
		{
			if (State != BehaviorNodeState::RUNNING)
			{
				State = BehaviorNodeState::RUNNING;
			}

			// Get State Object from BlackBoard
			auto SO = &BTree->GetBlackBoard()->SO;

			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		}

		private:
			void (*Action)(BehaviorTree*);
	};

	class FindRandomLocationTask : public Task<FindRandomLocationTask>
	{
		public:

		FindRandomLocationTask(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT)
						   {
						   		auto P = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition");
						   		auto Manager = ECS::Systems::EntitySystem::World();

						   		auto W = Manager->Lvl->GetWidth();
						   		auto H = Manager->Lvl->GetHeight();

						   		auto X = ER::Roll(0, 500);
						   		auto Y = ER::Roll(-2000, -200);

						   		P->SetData(EM::Vec3(X, Y, 0.0f));
						   };

			State = BehaviorNodeState::INVALID;
		}

		BehaviorNodeState Run()
		{
			if (State != BehaviorNodeState::RUNNING)
			{
				State = BehaviorNodeState::RUNNING;
			}

			// Get State Object from BlackBoard
			auto SO = &BTree->GetBlackBoard()->SO;

			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

		private:
			void (*Action)(BehaviorTree*);
	};
	*/

	// class MoveToTargetLocationTask : public Task<MoveToTargetLocationTask>
	// MoveToTargetLocationTask::MoveToTargetLocationTask(BehaviorTree* BT)
	// {
	// 	this->BTree = BT;
	// 	this->Action = [](BT::BehaviorTree* BT)
	// 					 {
	// 				   		auto P = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition");
	// 				   		auto Manager = ECS::Systems::EntitySystem::World();
	// 				   		auto ID = BT->GetBlackBoard()->GetComponent<ECS::eid32>("EID");

	// 				   		auto Target = P->GetData();
	// 				   		auto ai = ID->GetData();

	// 				   		auto TargetCartesian = EM::IsoToCartesian(Target.XY());
							
	// 						auto AI = &Manager->TransformSystem->Transforms[ai];
	// 				   		auto H = Manager->AttributeSystem->HealthComponents[ai].Health; 

	// 						// Find difference in positions	
	// 						Enjon::Math::Vec3 Difference = EM::Vec3::Normalize(EM::Vec3(TargetCartesian, Target.z) - 
	// 																			EM::Vec3(AI->CartesianPosition, AI->Position.z));
	// 						float speed = 5000.0f / (H + 0.0001f);

	// 						AI->VelocityGoal = Difference * 2.0f;
	// 					 };

	// 	State = BehaviorNodeState::INVALID;
	// }

	// MoveToTargetLocationTask::BehaviorNodeState Run()
	// {
	// 	if (State != BehaviorNodeState::RUNNING)
	// 	{
	// 		State = BehaviorNodeState::RUNNING;
	// 	}

	// 	// Get State Object from BlackBoard
	// 	auto SO = &BTree->GetBlackBoard()->SO;

	// 	Action(BTree);

	// 	State = BehaviorNodeState::SUCCESS;

	// 	SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

	// 	return BehaviorNodeState::SUCCESS;
	// };
}





#endif