#ifndef ENJON_BEHAVIORNODE_H
#define ENJON_BEHAVIORNODE_H

#include <cstdint>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <initializer_list>

#include "Utils/Errors.h"
#include "System/Internals.h"
#include "System/Types.h"

typedef Enjon::uint32 u32;
typedef Enjon::int32  i32;

namespace BT
{
	class BehaviorTree; 
	class BlackBoard;

	enum BehaviorNodeState { RUNNING, SUCCESS, FAILURE, INVALID };
	enum BehaviorNodeType 	{ LEAF, COMPOSITE, DECORATOR };

	class BehaviorNodeBase
	{
		public:

			virtual void Init() = 0; 
			virtual BehaviorNodeState Run() = 0;
			virtual u32 GetChildSize() = 0;
			virtual	std::vector<BehaviorNodeBase*> GetAllChildren() = 0; 
			virtual u32 SetIndicies(u32 I) = 0;
			virtual void Reset() = 0;
			virtual std::string String() = 0;
			virtual void AddChild(BehaviorNodeBase* B) = 0;

			inline BehaviorNodeState GetState() const { return State; }
			inline void SetState(BehaviorNodeState S) { State = S; }
			inline BlackBoard* GetBB() { return BB; }
			inline void SetParent(BehaviorNodeBase* P) { Parent = P; }
			u32 GetIndex() const { return TreeIndex; }

		public:
			BehaviorNodeType Type;

		protected:
			BehaviorNodeState State;
			BehaviorNodeBase* Parent;
			BehaviorTree* BTree;
			BlackBoard* BB;
			std::string Name;
			u32 TreeIndex;
	};

	class StateObject
	{
		public:
			std::vector<BehaviorNodeState> States;
			BehaviorNodeBase* CurrentNode;
	};

	class BlackBoardComponentBase
	{
		public:
			virtual void Init() = 0;
	};

	template <typename T>
	class BlackBoardComponent : public BlackBoardComponentBase
	{
		public:

			BlackBoardComponent(){}
			BlackBoardComponent(T Data)
			{
				this->InternalData.Value = Data;
			}

			void Init(){}

			// Get and set data
			inline T GetData() { return InternalData.Value; }
			inline void SetData(T t) { InternalData.Value = t; }

		protected:
			Enjon::Internals::Internal<T> InternalData;
	};

	class BlackBoard
	{
		public:
			BlackBoard(){}
			~BlackBoard()
			{
				RemoveComponents();
			}

			void AddComponent(std::string S, BlackBoardComponentBase* B)
			{
				Components[S] = B;
			}

			template <typename T>
			inline BlackBoardComponent<T>* GetComponent(std::string S) 
			{
				auto it = Components.find(S);
				if (it != Components.end()) return static_cast<BlackBoardComponent<T>*>(Components[S]);
				else Enjon::Utils::FatalError("BlackBoard Component Does Not Exist: " + S);
			}

			inline void RemoveComponents()
			{
				for (auto itr = Components.begin(); itr != Components.end(); ++itr)
				{
					auto val = (*itr).second;
					delete val;
				}
			}

		public:
			StateObject SO;
			std::unordered_map<std::string, BlackBoardComponentBase*> Components;
	};	

	// A wrapper for our nodes
	class BehaviorTree
	{
		public:

			BehaviorTree() : Root(nullptr), BB(nullptr) 
			{
				State = BehaviorNodeState::RUNNING;
			}

			void End() { SetTreeIndicies(); }

			~BehaviorTree(){}


			inline StateObject BehaviorTree::CreateStateObject()
			{
				StateObject SO;

				// Return SO if no root
				if (Root == nullptr) return SO;

				// Get total child size of tree by recursively calling get child size + Root
				auto S = Root->GetChildSize() + 1;

				// Push back states into SO
				for (u32 i = 0; i < S; i++)
				{
					SO.States.push_back(BehaviorNodeState::INVALID);
				}

				return SO;
			}

			inline BlackBoard CreateBlackBoard()
			{
				// Make SO
				auto SO = CreateStateObject();	

				// Make BB
				BlackBoard BB;

				// Put in BB
				BB.SO = SO;

				BB.SO.CurrentNode = this->Root;

				return BB;
			}

			inline void Run(BlackBoard* bb)
			{
				// HotSwap in BlackBoard to be operated on
				BB = bb;

				// Now run tree on BB
				Root->Run();
			}

			template <typename T>
			inline T* GetRoot() const { return static_cast<T*>(Root); }
			inline void SetRoot(BehaviorNodeBase* B) { Root = B; }
			inline BlackBoard* GetBlackBoard() { return BB; }
			inline BehaviorNodeState GetState() const { return State; }

		public:
			BehaviorNodeBase* Root;

		private:
			BlackBoard* BB;
			BehaviorNodeState State;

		private:
	
			inline void BehaviorTree::SetTreeIndicies()
			{
				u32 i = 0;
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

				// Call child's set indicies and get return value
				auto U = Child->SetIndicies(I + 1);
				return U;
			}

			void Reset()
			{
				// Get State Object from BlackBoard
				auto SO = &BTree->GetBlackBoard()->SO;
				auto SS = &SO->States;

				// Reset state
				SS->at(this->TreeIndex) = BehaviorNodeState::RUNNING;

				// Reset its child
				Child->Reset();
			}

		public:
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

			void AddChild(BehaviorNodeBase* N)
			{
				if (N) Children.push_back(N);
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
				auto S = &BTree->GetBlackBoard()->SO;
				S->States.at(this->TreeIndex) = BehaviorNodeState::RUNNING;
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

				u32 U = I;

				for(auto& C : Children) 
				{
					U++;
					U = C->SetIndicies(U);
				}

				return U;
			}

			inline std::vector<BehaviorNodeBase*> GetAllChildren() { return Children; }

		public:
			std::vector<BehaviorNodeBase*> Children;
			std::vector<BehaviorNodeBase*>::const_iterator Itr;
			BehaviorNodeBase* CurrentNode;
	};



}



#endif






