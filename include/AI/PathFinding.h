#ifndef ENJON_PATHFINDING_H
#define ENJON_PATHFINDING_H

#include <System/Containers.h>
#include <System/Types.h>
#include <deque>

#include "AI/SpatialHash.h"

namespace PathFinding {
	
	struct Node 
	{
		EM::Vec2 GridCoordinates;
		float GCost;
		float HCost;
		float WCost;
		float FCost;
		Enjon::uint32 Index;
		Node* Parent;
	};	

	template <typename T, typename K>
	inline T PopHeap(std::priority_queue<T, std::vector<T>, K>& Q)
	{
		T Val = Q.top();
		Q.pop();
		return Val;
	}

	template <typename T>
	inline bool SetFind(std::unordered_set<T>& S, T Val) 
	{
		auto it = S.find(Val);
		if (it != S.end()) return true;
		return false;
	}

	typedef Enjon::HeapItem<Node, float> HeapNode;

	inline Enjon::int32 CompareHeapNode(HeapNode* A, HeapNode* B)
	{
		if (A->PriorityValue == B->PriorityValue)  	return 0;
		if (A->PriorityValue >  B->PriorityValue) 	return 1;
		return -1;
	}

	struct CompareNode
	{
		inline bool operator() (Node& A, Node& B) {
			return A.FCost > B.FCost;
		}
	};

	inline bool operator==(Node& A, Node& B) {
		return A.Index == B.Index;	
	}

	inline bool operator!=(Node& A, Node& B) {
		return A.Index != B.Index;	
	}

	std::deque<Node> FindPath(SpatialHash::Grid* G, EM::Vec2& Start, EM::Vec2& End);

	Enjon::uint32 GetDistance(Node& A, Node& B);

	Node CreateNodeFromPosition(SpatialHash::Grid* G, EM::Vec2& Position);

	Node CreateNodeFromGridCoordinates(SpatialHash::Grid* G, EM::Vec2& GridCoordinates);

	std::vector<Node> GetNeighbors(SpatialHash::Grid* G, Node& CurrentNode);

	Node CreateNodeFromIndex(SpatialHash::Grid* G, Enjon::uint32 Index);
}

#endif