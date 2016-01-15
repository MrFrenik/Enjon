#ifndef SPATIALHASH_H
#define SPATIALHASH_H

#include <vector>
#include <set>
#include <stdlib.h>

#include <ECS/Entity.h>
#include <Math/Maths.h>
#include <Utils/Errors.h>

typedef Enjon::Math::Vec2 V2; 

const int CELL_SIZE = 256;

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
	} Grid; 

	void Init(Grid* grid, int width, int height); 
	int FindCell(Grid* grid, ECS::eid32 entity, const V2* position); 
	void ClearCells(Grid* grid);
	void GetNeighborCells(Grid* grid, int index, std::vector<ECS::eid32>* Entities);
}

#endif