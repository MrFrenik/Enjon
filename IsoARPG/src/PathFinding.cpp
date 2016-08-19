#include "PathFinding.h"

#define HOMEBREW 1

using namespace Enjon;

const int MAX_ITERATIONS = 2500;

namespace PathFinding {

#if !HOMEBREW
	Node CreateNodeFromPosition(SpatialHash::Grid* G, EM::Vec2& Position)
	{
		// Create node
		Node N = {SpatialHash::FindGridCoordinates(G, Position), 0, 0, 0, 0, 0, nullptr};
		
		// Set its index
		N.Index = SpatialHash::GetGridIndexFromCoordinates(G, N.GridCoordinates);

		return N;	
	}

	Node CreateNodeFromGridCoordinates(SpatialHash::Grid* G, EM::Vec2& GridCoordinates)
	{
		return {GridCoordinates, 0, 0, 0, 0, SpatialHash::GetGridIndexFromCoordinates(G, GridCoordinates), nullptr};
	}

	Node CreateNodeFromIndex(SpatialHash::Grid* G, Enjon::uint32 Index)
	{
		return {SpatialHash::FindGridCoordinatesFromIndex(G, Index), 0, 0, 0, 0, Index, nullptr};
	}

	std::deque<Node> FindPath(SpatialHash::Grid* G, EM::Vec2& Start, EM::Vec2& End)
	{
		std::deque<Node> Path;

		// Get start and end nodes
		Node StartNode = CreateNodeFromPosition(G, Start);
		Node EndNode = CreateNodeFromPosition(G, End);

		if (StartNode == EndNode) return Path;


		// Create open and closed sets as well set for random access of items in open set
		std::priority_queue<Node, std::vector<Node>, CompareNode> OpenSet;
		std::unordered_set<Enjon::uint32> OpenSetIndicies;
		std::unordered_set<Enjon::uint32> ClosedSet;

		// Add start node to open set
		OpenSet.push(StartNode);
		OpenSetIndicies.insert(StartNode.Index);

		// Keep processing while open set contains items
		while(OpenSet.size())
		{
			// Get current node to process and add to closed set
			Node CurrentNode = PopHeap<Node, CompareNode>(OpenSet);
			ClosedSet.insert(CurrentNode.Index);

			// Fill path if at endnode 
			if (CurrentNode == EndNode)
			{
				// Retrace path
				while (CurrentNode != StartNode)
				{
					Path.push_front(CurrentNode);

					// Get new node from current node's parent index
					CurrentNode = CreateNodeFromIndex(G, G->cells.at(CurrentNode.Index).ParentIndex);
				}

				// Reverse path
				// std::reverse(Path.begin(), Path.end());

				return Path;
			}

			// Otherwise, we need to get neighbors and process those
			for (Node& Neighbor : GetNeighbors(G, CurrentNode))
			{
				if (G->cells.at(Neighbor.Index).ObstructionValue >= 0.6f || SetFind<Enjon::uint32>(ClosedSet, Neighbor.Index))
				{
					continue;
				}

				// Get new movment cost
				Enjon::uint32 NewMovementCostToNeighbor = CurrentNode.GCost + GetDistance(CurrentNode, Neighbor);

				auto NeighborInOpenSet = SetFind<Enjon::uint32>(OpenSetIndicies, Neighbor.Index);

				if (NewMovementCostToNeighbor < Neighbor.GCost || !NeighborInOpenSet)
				{
					Neighbor.GCost = NewMovementCostToNeighbor;
					Neighbor.HCost = GetDistance(Neighbor, EndNode);
					Neighbor.FCost = Neighbor.GCost + Neighbor.HCost;
					// Neighbor.Parent = &CurrentNode;
					G->cells.at(Neighbor.Index).ParentIndex = CurrentNode.Index;

					if (!NeighborInOpenSet)
					{
						OpenSet.push(Neighbor);
						OpenSetIndicies.insert(Neighbor.Index);
					}
				}

			}
		}

		return Path;
	}

	std::vector<Node> GetNeighbors(SpatialHash::Grid* G, Node& CurrentNode)
	{
		std::vector<Node> Neighbors;

		auto CNC = CurrentNode.GridCoordinates.x;
		auto CNR = CurrentNode.GridCoordinates.y;

		auto Top = CNR + 1;
		auto Bottom = CNR - 1;
		auto Left = CNC - 1;
		auto Right = CNC + 1;

		bool TopValid = Top < G->rows;
		bool BottomValid = Bottom > 0;
		bool LeftValid = Left > 0;
		bool RightValid = Right < G->cols;
		
		// Top left
		if (LeftValid && TopValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Left, Top)));	

		// Top 
		if (TopValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(CNC, Top)));

		// Top Right
		if (RightValid && TopValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Right, Top)));

		// Right
		if (RightValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Right, CNR)));

		// Bottom Right
		if (RightValid && BottomValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Right, Bottom)));

		// Bottom
		if (BottomValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(CNC, Bottom)));

		// Bottom Left
		if (LeftValid && BottomValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Left, Bottom)));

		// Left
		if (LeftValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Left, CNR)));


		return Neighbors;
	}

	Enjon::uint32 GetDistance(Node& A, Node& B)
	{
		auto distX = static_cast<Enjon::uint32>(abs(A.GridCoordinates.x - B.GridCoordinates.x));
		auto distY = static_cast<Enjon::uint32>(abs(A.GridCoordinates.y - B.GridCoordinates.y));

		if (distX > distY)
			return 14 * distY + 10 * (distX - distY);
		else
			return 14 * distX + 10 * (distY - distX);
	}
#endif

#if HOMEBREW
	Node CreateNodeFromPosition(SpatialHash::Grid* G, EM::Vec2& Position)
	{
		// Create node
		Node N = {SpatialHash::FindGridCoordinates(G, Position), 0, 0, 0, 0, 0, nullptr};
		
		// Set its index
		N.Index = SpatialHash::GetGridIndexFromCoordinates(G, N.GridCoordinates);

		return N;	
	}

	Node CreateNodeFromGridCoordinates(SpatialHash::Grid* G, EM::Vec2& GridCoordinates)
	{
		return {GridCoordinates, 0, 0, 0, 0, SpatialHash::GetGridIndexFromCoordinates(G, GridCoordinates), nullptr};
	}

	Node CreateNodeFromIndex(SpatialHash::Grid* G, Enjon::uint32 Index)
	{
		return {SpatialHash::FindGridCoordinatesFromIndex(G, Index), 0, 0, 0, 0, Index, nullptr};
	}

	std::deque<Node> FindPath(SpatialHash::Grid* G, EM::Vec2& Start, EM::Vec2& End)
	{
		auto EndCount = 0;

		std::deque<Node> Path;

		// Get start and end nodes
		Node StartNode = CreateNodeFromPosition(G, Start);
		Node EndNode = CreateNodeFromPosition(G, End);

		if (StartNode == EndNode) return Path;

		// Create open and closed sets as well set for random access of items in open set
		Heap<Node, float> OpenSet(G->rows * G->cols, CompareHeapNode);
		std::unordered_set<Enjon::uint32> OpenSetIndicies;
		std::unordered_set<Enjon::uint32> ClosedSet;

		// Add start node to open set
		OpenSet.Add(HeapItem<Node, float>(StartNode, StartNode.FCost));
		OpenSetIndicies.insert(StartNode.Index);

		// Keep processing while open set contains items
		while(OpenSet.Count() && EndCount < MAX_ITERATIONS)
		{
			// Get current node to process and add to closed set
			Node CurrentNode = OpenSet.Pop().Item;
			ClosedSet.insert(CurrentNode.Index);

			// Fill path if at endnode 
			if (CurrentNode == EndNode)
			{
				// Retrace path
				while (CurrentNode != StartNode)
				{
					Path.push_front(CurrentNode);

					// Get new node from current node's parent index
					CurrentNode = CreateNodeFromIndex(G, G->cells.at(CurrentNode.Index).ParentIndex);
				}

				return Path;
			}

			// Otherwise, we need to get neighbors and process those
			for (Node& Neighbor : GetNeighbors(G, CurrentNode))
			{
				if (G->cells.at(Neighbor.Index).ObstructionValue >= 1.0f || SetFind<Enjon::uint32>(ClosedSet, Neighbor.Index))
				{
					continue;
				}

				// Get new movment cost
				Enjon::uint32 NewMovementCostToNeighbor = CurrentNode.GCost + GetDistance(CurrentNode, Neighbor) + CurrentNode.WCost;

				auto NeighborInOpenSet = SetFind<Enjon::uint32>(OpenSetIndicies, Neighbor.Index);

				if (NewMovementCostToNeighbor < Neighbor.GCost || !NeighborInOpenSet)
				// if (!NeighborInOpenSet)
				{
					Neighbor.GCost = NewMovementCostToNeighbor;
					Neighbor.HCost = GetDistance(Neighbor, EndNode);
					Neighbor.WCost = G->cells.at(Neighbor.Index).ObstructionValue * 100.0f;
					Neighbor.FCost = Neighbor.GCost + Neighbor.HCost;
					// Neighbor.Parent = &CurrentNode;
					G->cells.at(Neighbor.Index).ParentIndex = CurrentNode.Index;

					if (!NeighborInOpenSet)
					{
						OpenSet.Add(HeapItem<Node, float>(Neighbor, Neighbor.FCost));
						OpenSetIndicies.insert(Neighbor.Index);
					}
				}

			}

			EndCount++;
		}



		return Path;
	}

	std::vector<Node> GetNeighbors(SpatialHash::Grid* G, Node& CurrentNode)
	{
		std::vector<Node> Neighbors;

		auto CNC = CurrentNode.GridCoordinates.x;
		auto CNR = CurrentNode.GridCoordinates.y;

		auto Top = CNR + 1;
		auto Bottom = CNR - 1;
		auto Left = CNC - 1;
		auto Right = CNC + 1;

		bool TopValid = Top < G->rows;
		bool BottomValid = Bottom > 0;
		bool LeftValid = Left > 0;
		bool RightValid = Right < G->cols;

		auto TopLNeighborIndex 		= (Top * G->cols + CNC);
		auto BottomNeighborIndex 	= (Bottom * G->cols + CNC);
		auto RightNeighborIndex 	= (CNR * G->cols + Right);
		auto LeftNeightborIndex 	= (CNR * G->cols + Left);
		
		// Top left
		if (LeftValid && TopValid && G->cells.at(TopLNeighborIndex).ObstructionValue < 1.0f && G->cells.at(LeftNeightborIndex).ObstructionValue < 1.0f) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Left, Top)));	

		// Top 
		if (TopValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(CNC, Top)));

		// Top Right
		if (RightValid && TopValid && G->cells.at(TopLNeighborIndex).ObstructionValue < 1.0f && G->cells.at(RightNeighborIndex).ObstructionValue < 1.0f) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Right, Top)));

		// Right
		if (RightValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Right, CNR)));

		// Bottom Right
		if (RightValid && BottomValid && G->cells.at(BottomNeighborIndex).ObstructionValue < 1.0f && G->cells.at(RightNeighborIndex).ObstructionValue < 1.0f) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Right, Bottom)));

		// Bottom
		if (BottomValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(CNC, Bottom)));

		// Bottom Left
		if (LeftValid && BottomValid && G->cells.at(BottomNeighborIndex).ObstructionValue < 1.0f && G->cells.at(LeftNeightborIndex).ObstructionValue < 1.0f) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Left, Bottom)));

		// Left
		if (LeftValid) Neighbors.emplace_back(CreateNodeFromGridCoordinates(G, EM::Vec2(Left, CNR)));


		return Neighbors;
	}

	Enjon::uint32 GetDistance(Node& A, Node& B)
	{
		auto distX = static_cast<Enjon::uint32>(abs(A.GridCoordinates.x - B.GridCoordinates.x));
		auto distY = static_cast<Enjon::uint32>(abs(A.GridCoordinates.y - B.GridCoordinates.y));

		if (distX > distY)
			return 14 * distY + 10 * (distX - distY);
		else
			return 14 * distX + 10 * (distY - distX);
	}	
#endif

}
