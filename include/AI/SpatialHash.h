#ifndef ENJON_SPATIALHASH_H
#define ENJON_SPATIALHASH_H

#include <vector>
#include <set>
#include <stdlib.h>

#include <ECS/Entity.h>
#include <Math/Maths.h>
#include <Utils/Errors.h>
#include <Physics/AABB.h>

typedef Enjon::Vec2 V2; 

const int CELL_SIZE = 64;

namespace SpatialHash { 

	typedef struct 
	{
		std::vector<Enjon::uint32> entities;
		Enjon::uint32 ParentIndex;
		float ObstructionValue;
	} Cell;

	typedef struct                 
	{
		int rows;
		int cols;
		int CellSize;
		Enjon::Vec2 Origin;
		std::vector<Cell> cells; 
		std::vector<Enjon::uint32> dirtyCells;
	} Grid; 

	/* Inits spatial grid based on width, height of level and given cell size */ 
	void Init(Grid* grid, int width, int height, int cell_size = CELL_SIZE); 

	/* Finds particular cell that a given entity belongs to based on its position */
	Enjon::Vec2 FindGridCoordinates(Grid* grid, V2& position); 

	/* Overloaded function that finds particular cell that a given entity belongs to based on its AABB (preferred method) */
	std::vector<Enjon::uint32> FindCell(Grid* grid, Enjon::uint32 entity, const Enjon::Physics::AABB* AABB); 

	/* Clears all entity vectors from every cell in the spatial grid */
	void ClearCells(Grid* grid);

	/* Finds all neighboring cells to a given entitiy's cell and stores those in a passed in entities vector */
	void GetNeighborCells(Grid* grid, int index, std::vector<Enjon::uint32>* Entities);

	void DrawGrid(Grid* G, Enjon::SpriteBatch* Batch, Enjon::Vec2& Position = Enjon::Vec2(0.0f));

	void DrawActiveCell(Grid* G, Enjon::SpriteBatch* Batch, Enjon::Vec2& Position);

	Enjon::Vec4 GetCellDimensions(Grid* G, Enjon::Vec2& Cell);

	Enjon::Vec2 FindGridCoordinatesFromIndex(Grid* G, Enjon::uint32 Index);

	void FindCells(Grid* G, Enjon::uint32 Entity, const EP::AABB* AABB, Enjon::Vec4* CellRange, float ObstructionValue);

	std::vector<Enjon::uint32> GetEntitiesFromCells(Grid* G, Enjon::Vec4& Cells);

	Enjon::uint32 GetGridIndexFromCoordinates(Grid* G, Enjon::Vec2& Coordinates);
}

#endif