#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <unordered_map>
#include <vector>

// Need to keep track of K/V pairs
// Need the keys to be strings and the pairs to be whatever value

class BlackBoardComponentBase
{
	public:
		virtual void Init() 	= 0;
};

template <typename T>
class BlackBoardComponent : public BlackBoardComponentBase
{
	public:

		BlackBoardComponent(){}
		BlackBoardComponent(T Data)
		{
			this->Data = Data;
		}

		virtual void Init(){}

		inline T GetData() { return Data; }
		inline void SetData(T t) { Data = t; }

	protected:
		T Data;
};


namespace BT
{
	class BlackBoard
	{
		public:
			BlackBoard(){}
			~BlackBoard()
			{
				for (auto itr = Components.begin(); itr != Components.end(); itr++)
				{
					delete itr->second;
					Components.erase(itr);
				}
			}

			void AddComponent(std::string S, BlackBoardComponentBase* B)
			{
				Components[S] = B;
			}

			inline BlackBoardComponentBase* GetComponent(std::string S) { return Components[S]; }

		private:
			std::unordered_map<std::string, BlackBoardComponentBase*> Components;
	};	
}

#endif