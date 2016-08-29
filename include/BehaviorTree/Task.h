#ifndef ENJON_TASK_H
#define ENJON_TASK_H

#include "BehaviorNode.h"
#include "Math/Maths.h"
#include "ECS/ComponentSystems.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"

namespace BT
{
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
			SO->CurrentNode = this;

			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		}

		std::string String()
		{
			return std::string("SimpleTask");
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

						   		P->SetData(EM::Vec3(W, H, 0.0f));
						   };

			State = BehaviorNodeState::INVALID;
		}

		std::string String()
		{
			return std::string("FindRandomLocationTask");
		}

		BehaviorNodeState Run()
		{
			if (State != BehaviorNodeState::RUNNING)
			{
				State = BehaviorNodeState::RUNNING;
			}

			// Get State Object from BlackBoard
			auto SO = &BTree->GetBlackBoard()->SO;
			SO->CurrentNode = this;

			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

		private:
			void (*Action)(BehaviorTree*);
	};

	class SetPlayerLocationAsTargetTask : public Task<SetPlayerLocationAsTargetTask>
	{
		public:

		SetPlayerLocationAsTargetTask(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT)
						   {
						   		auto P = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition");
						   		auto Manager = ECS::Systems::EntitySystem::World();
						   		auto Player = Manager->Player;

						   		P->SetData(Manager->TransformSystem->Transforms[Player].Position);
						   };

			State = BehaviorNodeState::INVALID;
		}

		std::string String()
		{
			return std::string("SetPlayerLocationAsTargetTask");
		}

		BehaviorNodeState Run()
		{
			// Get State Object from BlackBoard
			auto SO = &BTree->GetBlackBoard()->SO;
			SO->CurrentNode = this;

			if (State != BehaviorNodeState::RUNNING)
			{
				State = BehaviorNodeState::RUNNING;
			}

			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

		private:
			void (*Action)(BehaviorTree*);
	};

	class StopMovingTask : public Task<StopMovingTask>
	{
		public:

		StopMovingTask(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT)
							 {
						   		auto Manager = ECS::Systems::EntitySystem::World();
						   		auto ID = BT->GetBlackBoard()->GetComponent<ECS::eid32>("EID")->GetData();
						   		Manager->TransformSystem->Transforms[ID].Velocity = EM::Vec3(0.0f, 0.0f, 0.0f);
							 };

			State = BehaviorNodeState::INVALID;
		}

		std::string String()
		{
			return std::string("StopMovingTask");
		}

		BehaviorNodeState Run()
		{
			// Get State Object from BlackBoard
			auto SO = &BTree->GetBlackBoard()->SO;
			SO->CurrentNode = this;

			if (State != BehaviorNodeState::RUNNING)
			{
				State = BehaviorNodeState::RUNNING;
			}


			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

		private:
			void (*Action)(BehaviorTree*);
	};

	class MoveToTargetLocationTask : public Task<MoveToTargetLocationTask>
	{
		public:

		MoveToTargetLocationTask(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT)
							 {
						   		auto Target = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition")->GetData();
						   		auto Manager = ECS::Systems::EntitySystem::World();
						   		auto ID = BT->GetBlackBoard()->GetComponent<ECS::eid32>("EID")->GetData();
								auto Position = &Manager->TransformSystem->Transforms[ID].Position;

								// Find difference in positions	
								Enjon::Math::Vec3 Direction = EM::Vec3::Normalize(Target - *Position);

								// Store the speed somewhere in the transform system
								Manager->TransformSystem->Transforms[ID].Velocity = Direction * 2.0f;
							 };

			State = BehaviorNodeState::INVALID;
		}

		std::string String()
		{
			return std::string("MoveToTargetLocationTask");
		}

		BehaviorNodeState Run()
		{
			// Get State Object from BlackBoard
			auto SO = &BTree->GetBlackBoard()->SO;
			SO->CurrentNode = this;

			if (State != BehaviorNodeState::RUNNING)
			{
				State = BehaviorNodeState::RUNNING;
			}


			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

		private:
			void (*Action)(BehaviorTree*);
	};
}





#endif