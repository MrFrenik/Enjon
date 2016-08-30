#ifndef ENJON_TASK_H
#define ENJON_TASK_H

#include "BehaviorNode.h"
#include "Math/Maths.h"
#include "ECS/ComponentSystems.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/AttributeSystem.h"
#include "System/Internals.h"

namespace BT
{
	// Base Task - not to be used
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

			void AddChild(BehaviorNodeBase* B)
			{}

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
			Type = BehaviorNodeType::LEAF;
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

	class FindRandomLocation : public Task<FindRandomLocation>
	{
		public:

		FindRandomLocation(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT)
						   {
						   		auto P = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition");
						   		auto Manager = ECS::Systems::EntitySystem::World();

						   		auto W = static_cast<Enjon::int32>(Manager->Lvl->GetWidth() / 2.0f);
						   		auto H = static_cast<Enjon::int32>(Manager->Lvl->GetHeight() / 2.0f);

						   		P->SetData(EM::Vec3(ER::Roll(-W, W), ER::Roll(-H, H), 0.0f));
						   };

			State = BehaviorNodeState::INVALID;
			Type = BehaviorNodeType::LEAF;
		}

		std::string String()
		{
			return std::string("FindRandomLocation");
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

	class SetViewVectorToTarget : public Task<SetViewVectorToTarget>
	{
		public:
			SetViewVectorToTarget(BehaviorTree* BT)
			{
				this->BTree = BT;
				this->Action = [](BT::BehaviorTree* BT)
							   {
							   		auto P = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition")->GetData();
							   		auto ID = BT->GetBlackBoard()->GetComponent<Enjon::uint32>("EID")->GetData();
							   		auto Manager = ECS::Systems::EntitySystem::World();
							   		auto Position = &Manager->TransformSystem->Transforms[ID].Position;

							   		// Calculate view vector and set
							   		auto View = EM::Vec2::Normalize((P - *Position).XY());

							   		Manager->TransformSystem->Transforms[ID].ViewVector = View;
							   };

				State = BehaviorNodeState::INVALID;
				Type = BehaviorNodeType::LEAF;
			}

			std::string String()
			{
				return std::string("SetViewVectorToTarget");
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

	class SetPlayerLocationAsTarget : public Task<SetPlayerLocationAsTarget>
	{
		public:

		SetPlayerLocationAsTarget(BehaviorTree* BT)
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
			Type = BehaviorNodeType::LEAF;
		}

		std::string String()
		{
			return std::string("SetPlayerLocationAsTarget");
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

	class StopMoving : public Task<StopMoving>
	{
		public:

		StopMoving(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT)
							 {
						   		auto Manager = ECS::Systems::EntitySystem::World();
						   		auto ID = BT->GetBlackBoard()->GetComponent<ECS::eid32>("EID")->GetData();
						   		Manager->TransformSystem->Transforms[ID].Velocity = EM::Vec3(0.0f, 0.0f, 0.0f);
							 };

			State = BehaviorNodeState::INVALID;
			Type = BehaviorNodeType::LEAF;
		}

		std::string String()
		{
			return std::string("StopMoving");
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

	class MoveToTargetLocation : public Task<MoveToTargetLocation>
	{
		public:

		MoveToTargetLocation(BehaviorTree* BT)
		{
			this->BTree = BT;
			this->Action = [](BT::BehaviorTree* BT, Enjon::Internals::EFloat& Speed)
							 {
						   		auto Target = BT->GetBlackBoard()->GetComponent<EM::Vec3>("TargetPosition")->GetData();
						   		auto Manager = ECS::Systems::EntitySystem::World();
						   		auto ID = BT->GetBlackBoard()->GetComponent<ECS::eid32>("EID")->GetData();
								auto Position = &Manager->TransformSystem->Transforms[ID].Position;

								// Find difference in positions	
								Enjon::Math::Vec3 Direction = EM::Vec3::Normalize(Target - *Position);

								// Store the speed somewhere in the transform system
								Manager->TransformSystem->Transforms[ID].Velocity = Direction * Speed.Value;
							 };

			State = BehaviorNodeState::INVALID;
			Type = BehaviorNodeType::LEAF;

			// Default speed value
			Speed.Value = 1.0f;
		}

		std::string String()
		{
			return std::string("MoveToTargetLocation");
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


			Action(BTree, Speed);

			State = BehaviorNodeState::SUCCESS;

			SO->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

		public:
			Enjon::Internals::EFloat Speed;	

		private:
			void (*Action)(BehaviorTree*, Enjon::Internals::EFloat& Speed);
	};
}





#endif