#include "SpatialHash.h"
#include <stdio.h>

namespace SpatialHash {

	void Init(Grid* grid, int width, int height, int cell_size)
	{
		if (grid == nullptr) Enjon::Utils::FatalError("SPATIALHASH::INIT::Cannot operate on null data.");

		grid->rows = ceil(height / cell_size);
		grid->cols = ceil(width / cell_size);

		// Allocate correct memory size for cells
		grid->cells.resize(grid->rows * grid->cols + 1);
	}

	int FindCell(Grid* grid, ECS::eid32 entity, const V2* position, int cell_size)
	{
		int posX = -position->x;
		int posY = -position->y; 

		int row = floor(posY / cell_size);
		int col = floor(posX / cell_size);

		int index = row * grid->cols + col;
		int max = grid->rows * grid->cols;
		if (index < 0) index = 0;
		if (index > max - 1) index = max - 1;

		grid->cells[index].entities.push_back(entity);

		return index;
	}

	std::vector<ECS::eid32> FindCell(Grid* grid, ECS::eid32 entity, const Enjon::Physics::AABB* AABB, int cell_size)
	{
		std::vector<ECS::eid32> entities;

		// Get individual elements of min and max and negate them to bring them into positive cell space
		int min_y = -AABB->Min.y;	
		int min_x = -AABB->Min.x;	
		int max_y = -AABB->Max.y;	
		int max_x = -AABB->Max.x;	

		// Min row and col
		int min_row = floor(min_y / cell_size);
		int min_col = floor(min_x / cell_size);

		// Max row and col
		int max_row = floor(max_y / cell_size);
		int max_col = floor(max_x / cell_size);

		int max_allowed_index = grid->rows * grid->cols;

		// Find all cells that entity belongs to and place it in them
		for (int i = max_row; i <= min_row; i++)
		{
			for (int j = max_col; j <= min_col; j++)
			{
				int index = i * grid->cols + j;
				if (index < 0 || index >= max_allowed_index) continue;

				// Add all entities in this cell into entities vector for return 
				entities.insert(entities.end(), grid->cells[index].entities.begin(), grid->cells[index].entities.end());

				// Then push back this entity
				grid->cells[index].entities.push_back(entity);
			}
		}

		return entities;
	}

	void ClearCells(Grid* grid)
	{
		if (grid == nullptr) Enjon::Utils::FatalError("SPATIALHASH::CLEARCELLS::Cannot operate on null data.");

		for (Cell& cell : grid->cells)
		{ 
			if (!cell.entities.empty()) cell.entities.clear();
		}

	}

	// TODO(John): Create a pair checking function to maintain and update pairs of entities that have already been checked with one another
	// NOTE(John): As of now, this is incredibly too slow to work...
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
		if (Top >= 0)		   if (!grid->cells[Top].entities.empty())  			Entities->insert(Entities->end(), grid->cells[Top].entities.begin(), grid->cells[Top].entities.end()); 
		if (Bottom < max)	   if (!grid->cells[Bottom].entities.empty()) 			Entities->insert(Entities->end(), grid->cells[Bottom].entities.begin(), grid->cells[Bottom].entities.end());
		if (TopLeft >= 0)	   if (!grid->cells[TopLeft].entities.empty()) 			Entities->insert(Entities->end(), grid->cells[TopLeft].entities.begin(), grid->cells[TopLeft].entities.end());
		if (TopRight >= 0)	   if (!grid->cells[TopRight].entities.empty()) 		Entities->insert(Entities->end(), grid->cells[TopRight].entities.begin(), grid->cells[TopRight].entities.end());
		if (Left >= 0)		   if (!grid->cells[Left].entities.empty()) 			Entities->insert(Entities->end(), grid->cells[Left].entities.begin(), grid->cells[Left].entities.end());
		if (Right < max)	   if (!grid->cells[Right].entities.empty()) 			Entities->insert(Entities->end(), grid->cells[Right].entities.begin(), grid->cells[Right].entities.end());
		if (BottomLeft < max)  if (!grid->cells[BottomLeft].entities.empty()) 		Entities->insert(Entities->end(), grid->cells[BottomLeft].entities.begin(), grid->cells[BottomLeft].entities.end());
		if (BottomRight < max) if (!grid->cells[BottomRight].entities.empty()) 		Entities->insert(Entities->end(), grid->cells[BottomRight].entities.begin(), grid->cells[BottomRight].entities.end());
	}
}