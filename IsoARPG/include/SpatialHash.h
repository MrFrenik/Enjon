#ifndef SPATIALHASH_H
#define SPATIALHASH_H

#include <vector>
#include <set>
#include <stdlib.h>

#include <ECS/Entity.h>
#include <Math/Maths.h>
#include <Utils/Errors.h>
#include <Physics/AABB.h>

typedef Enjon::Math::Vec2 V2; 

const int CELL_SIZE = 64;

namespace SpatialHash { 

	typedef struct 
	{
		std::vector<ECS::eid32> entities;
	} Cell;

	typedef struct                 
	{
		int rows;
		int cols;
		std::vector<Cell> cells; 
		std::vector<Enjon::uint32> dirtyCells;
	} Grid; 

	/* Inits spatial grid based on width, height of level and given cell size */ 
	void Init(Grid* grid, int width, int height, int cell_size = CELL_SIZE); 

	/* Finds particular cell that a given entity belongs to based on its position */
	int FindCell(Grid* grid, ECS::eid32 entity, const V2* position, int cell_size = CELL_SIZE); 

	/* Overloaded function that finds particular cell that a given entity belongs to based on its AABB (preferred method) */
	std::vector<ECS::eid32> FindCell(Grid* grid, ECS::eid32 entity, const Enjon::Physics::AABB* AABB, int cell_size = CELL_SIZE); 

	/* Clears all entity vectors from every cell in the spatial grid */
	void ClearCells(Grid* grid);

	/* Finds all neighboring cells to a given entitiy's cell and stores those in a passed in entities vector */
	void GetNeighborCells(Grid* grid, int index, std::vector<ECS::eid32>* Entities);
}

#endif