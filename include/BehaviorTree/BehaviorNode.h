#ifndef BEHAVIORNODE_H
#define BEHAVIORNODE_H

#include <cstdint>
#include <vector>
#include <iostream>
#include <initializer_list>

#include "BlackBoard.h"

typedef uint32_t u32;
typedef int32_t  i32;


namespace BT
{
	class BehaviorTree; 

	enum BehaviorNodeState { RUNNING, SUCCESS, FAILURE, INVALID };

	class BehaviorNodeBase
	{
		public:

			virtual void Init() 									= 0; 
			virtual BehaviorNodeState Run()							= 0;
			virtual u32 GetChildSize() 								= 0;
			virtual	std::vector<BehaviorNodeBase*> GetAllChildren()	= 0; 
			virtual u32 SetIndicies(u32 I)  						= 0;
			virtual void Reset() 									= 0;

			inline BehaviorNodeState GetState() const { return State; }
			inline void SetState(BehaviorNodeState S) { State = S; }
			inline BlackBoard* GetBB() { return BB; }
			inline void SetParent(BehaviorNodeBase* P) { Parent = P; }
			u32 GetIndex() const { return TreeIndex; }


		protected:
			BehaviorNodeState State;
			BehaviorNodeBase* Parent;
			BehaviorTree* BTree;
			BlackBoard* BB;
			u32 TreeIndex;
	};

	struct StateObject
	{
		std::vector<BehaviorNodeState> States;
		BehaviorNodeBase* CurrentNode;
	};

	// A wrapper for our nodes
	class BehaviorTree
	{
		public:

			BehaviorTree(BehaviorNodeBase* R, BlackBoard* bb) : Root(R), BB(bb) 
			{
				// Set up BB with empty state object
				StateObject* SO = CreateStateObject();

				// Put in BB
				BB->AddComponent("States", new BlackBoardComponent<StateObject*>(SO));

				State = BehaviorNodeState::RUNNING;
			}

			BehaviorTree() : Root(nullptr), BB(new BlackBoard()) 
			{
				// Set up BB with empty state object
				StateObject* SO = CreateStateObject();

				// Put in BB
				BB->AddComponent("States", new BlackBoardComponent<StateObject*>(SO));

				State = BehaviorNodeState::RUNNING;
			}

			void End() { SetTreeIndicies(); }

			~BehaviorTree(){}

			inline StateObject* BehaviorTree::CreateStateObject()
			{
				StateObject* SO = new StateObject();

				// Return SO if no root
				if (Root == nullptr) return SO;

				// Get total child size of tree by recursively calling get child size + Root
				auto S = Root->GetChildSize() + 1;

				// Push back states into SO
				for (u32 i = 0; i < S; i++)
				{
					SO->States.push_back(BehaviorNodeState::INVALID);
				}

				return SO;
			}

			inline BlackBoard* CreateBlackBoard()
			{
				// Make SO
				auto SO = CreateStateObject();	

				BlackBoard* BB = new BlackBoard();

				// Put in BB
				BB->AddComponent("States", new BlackBoardComponent<StateObject*>(SO));

				return BB;
			}

			inline void Run(BlackBoard* bb)
			{
				// HotSwap in BlackBoard to be operated on
				BB = bb;
				auto S = static_cast<BlackBoardComponent<StateObject*>*>(BB->GetComponent("States"));

				// Now run tree on BB
				Root->Run();
			}

			template <typename T>
			inline T* GetRoot() const { return static_cast<T*>(Root); }
			inline void SetRoot(BehaviorNodeBase* B) { Root = B; }
			inline BlackBoard* GetBlackBoard() { return BB; }
			inline BehaviorNodeState GetState() const { return State; }


		private:
			BehaviorNodeBase* Root;
			BlackBoard* BB;
			BehaviorNodeState State;

		private:
	
			inline void BehaviorTree::SetTreeIndicies()
			{
				u32 i = 0;
				std::cout << "Root: " << i << std::endl;
				auto R = Root->SetIndicies(i);
			}
	};


	template <typename T>
	class BehaviorNode : public BehaviorNodeBase
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
	};

	template <typename T>
	class Decorator : public BehaviorNode<Decorator<T>>
	{
		public:

			void AddChild(BehaviorNodeBase* N)
			{
				if (N) 
				{
					N->SetParent(this);
					Child = N;
				}
			}

			inline u32 GetChildSize() 
			{ 
				u32 S = 0;

				if (Child != nullptr)
				{
					S += Child->GetChildSize();
					S += 1;
				}

				return S;
			}

			inline std::vector<BehaviorNodeBase*> GetAllChildren() 
			{ 
				std::vector<BehaviorNodeBase*> C; 
				C.push_back(Child); 
				return C; 
			}

			inline u32 SetIndicies(u32 I) 
			{ 
				this->TreeIndex = I; 
				std::cout << "Dec: " << I << std::endl;

				// Call child's set indicies and get return value
				auto U = Child->SetIndicies(I + 1);
				return U;
			}

			void Reset()
			{
				// Get State Object from BlackBoard
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				// Reset state
				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;

				// Reset its child
				Child->Reset();
			}

		protected:
			BehaviorNodeBase* Child;

	};

	template <typename T>
	class Composite : public BehaviorNode<Composite<T>>
	{
		public:

			Composite(){}
			Composite(std::initializer_list<BehaviorNodeBase*> L)
			{
				for (auto N : L)
				{
					N->SetParent(this);
					Children.push_back(N);
				}
			}

			void Init()
			{
				Itr = Children.begin();
				BehaviorNodeBase* CurrentNode = nullptr;
				State = BehaviorNodeState::INVALID;
			}

			Composite* AddChild(BehaviorNodeBase* N)
			{
				if (N) Children.push_back(N);

				return this;
			}

			void AddChildren(std::initializer_list<BehaviorNodeBase*> V)
			{
				for (auto N : V)
				{
					V->SetParent(this);
					Children.insert(N);
				}
			}

			BehaviorNodeState Run()
			{
				return static_cast<T*>(this)->Run();
			}

			void Reset()
			{
				auto S = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				S->GetData()->States.at(this->TreeIndex) = BehaviorNodeState::RUNNING;
				Itr = Children.begin();
		
				// Reset all its children		
				for (auto& C : Children)
				{
					C->Reset();
				}
			
				CurrentNode = *Itr;
				State = BehaviorNodeState::RUNNING;
			}

			inline const std::vector<BehaviorNodeBase*>* GetChildren() { return &Children; }

			inline u32 GetChildSize() 
			{ 
				u32 S = 0;

				for (auto C : Children)
				{
					S += C->GetChildSize();
				}

				S += Children.size();

				return S;
			}

			inline u32 SetIndicies(u32 I)
			{
				this->TreeIndex = I;
				std::cout << "Comp: " << I << std::endl;

				u32 U = I;

				for(auto& C : Children) 
				{
					U++;
					U = C->SetIndicies(U);
				}

				return U;
			}

			inline std::vector<BehaviorNodeBase*> GetAllChildren() { return Children; }

		protected:
			std::vector<BehaviorNodeBase*> Children;
			std::vector<BehaviorNodeBase*>::const_iterator Itr;
			BehaviorNodeBase* CurrentNode;
	};

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
				auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));
				auto SS = &SO->GetData()->States;

				// Reset state
				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;
			}

			inline u32 GetChildSize() { return 0; }

			inline u32 SetIndicies(u32 I) { this->TreeIndex = I; std::cout << "Task: " << I << std::endl; return I; }

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
			auto SO = static_cast<BlackBoardComponent<StateObject*>*>(BTree->GetBlackBoard()->GetComponent("States"));

			Action(BTree);

			State = BehaviorNodeState::SUCCESS;

			SO->GetData()->States.at(this->TreeIndex) = BehaviorNodeState::SUCCESS;	

			return BehaviorNodeState::SUCCESS;
		};

	private:
		void (*Action)(BehaviorTree*);
	};


}



#endif






