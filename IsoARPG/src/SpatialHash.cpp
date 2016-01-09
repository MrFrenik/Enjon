#include "SpatialHash.h"
#include <stdio.h>

namespace SpatialHash {

	void Init(Grid* grid, int width, int height)
	{
		if (grid == nullptr) Enjon::Utils::FatalError("SPATIALHASH::INIT::Cannot operate on null data.");

		grid->rows = ceil(height / CELL_SIZE);
		grid->cols = ceil(width / CELL_SIZE);

		// Allocate correct memory size for cells
		grid->cells.resize(grid->rows * grid->cols + 1);
	}

	int FindCell(Grid* grid, ECS::eid32 entity, const V2* position)
	{
		int posX = -position->x;
		int posY = -position->y; 

		int row = floor(posY / CELL_SIZE);
		int col = floor(posX / CELL_SIZE);

		int index = row * grid->cols + col;
		int max = grid->rows * grid->cols;
		if (index < 0) index = 0;
		if (index > max - 1) index = max - 1;

		grid->cells[index].entities.push_back(entity);

		return index;
	}

	void ClearCells(Grid* grid)
	{
		if (grid == nullptr) Enjon::Utils::FatalError("SPATIALHASH::INIT::Cannot operate on null data.");

		for (Cell& cell : grid->cells)
		{ 
			if (!cell.entities.empty()) cell.entities.clear();
		}

	}

	// TODO(John): Create a pair checking function to maintain and update pairs of entities that have already been checked with one another
	
	void GetNeighborCells(Grid* grid, int index, std::vector<ECS::eid32>* Entities)
	{
		// Grab all neighbor cell indexes	
		int Top = index - grid->cols; 
		int TopLeft = Top - 1; 
		int TopRight = Top + 1; 
		int Left = index - 1;
		int Right = index + 1;
		int BottomLeft = index + grid->cols - 1;
		int Bottom = index + grid->cols;
		int BottomRight = index + grid->cols + 1;
		int max = grid->rows * grid->cols;

		// If valid, append to entities
		if (Top >= 0)		   { if (!grid->cells[Top].entities.empty()) { for (ECS::eid32& entity : grid->cells[Top].entities)		    			{ Entities->push_back(entity); }}} 
		if (Bottom < max)	   { if (!grid->cells[Bottom].entities.empty()) { for (ECS::eid32& entity : grid->cells[Bottom].entities)	   			{ Entities->push_back(entity); }}}
		if (TopLeft >= 0)	   { if (!grid->cells[TopLeft].entities.empty()) { for (ECS::eid32& entity : grid->cells[TopLeft].entities)	    		{ Entities->push_back(entity); }}}
		if (TopRight >= 0)	   { if (!grid->cells[TopRight].entities.empty()) { for (ECS::eid32& entity : grid->cells[TopRight].entities)   		{ Entities->push_back(entity); }}}
		if (Left >= 0)		   { if (!grid->cells[Left].entities.empty()) { for (ECS::eid32& entity : grid->cells[Left].entities)	    			{ Entities->push_back(entity); }}}
		if (Right < max)	   { if (!grid->cells[Right].entities.empty()) { for (ECS::eid32& entity : grid->cells[Right].entities)	    			{ Entities->push_back(entity); }}}
		if (BottomLeft < max)  { if (!grid->cells[BottomLeft].entities.empty()) { for (ECS::eid32& entity : grid->cells[BottomLeft].entities)  		{ Entities->push_back(entity); }}}
		if (BottomRight < max) { if (!grid->cells[BottomRight].entities.empty()) { for (ECS::eid32& entity : grid->cells[BottomRight].entities) 	{ Entities->push_back(entity); }}}
	}
}