#include "SpatialHash.h"
#include <stdio.h>

#include <Graphics/SpriteBatch.h>
#include <IO/ResourceManager.h>


namespace SpatialHash {

	/* Inits spatial grid based on width, height of level and given cell size */ 
	void Init(Grid* grid, int width, int height, int cell_size)
	{
		if (grid == nullptr) Enjon::Utils::FatalError("SPATIALHASH::INIT::Cannot operate on null data.");

		grid->rows = ceil(height / cell_size);
		grid->cols = ceil(width / cell_size);

		// Allocate correct memory size for cells
		grid->cells.resize(grid->rows * grid->cols + 1);

		// Set origin of grid
		// This needs to be passed in by world that creates it
		grid->Origin = EM::Vec2(CELL_SIZE / 2.0f, 0.0f);
	}

	/* Finds particular cell that a given entity belongs to based on its position */
	EM::Vec2 FindCellCoordinates(Grid* grid, const V2* position, int cell_size)
	{
		int posX = position->x;
		int posY = position->y; 

		int row = floor(posY / cell_size);
		int col = floor(posX / cell_size);

		int index = row * grid->cols + col;
		int max = grid->rows * grid->cols;
		if (index < 0) index = 0;
		if (index > max - 1) index = max - 1;


		return EM::Vec2(col, row);
	}

	/* Overloaded function that finds particular cell that a given entity belongs to based on its AABB (preferred method) */
	std::vector<ECS::eid32> FindCell(Grid* grid, ECS::eid32 entity, const Enjon::Physics::AABB* AABB, int cell_size)
	{
		std::vector<ECS::eid32> entities;

		// AABB elements
		int min_y = AABB->Min.y;	
		int min_x = AABB->Min.x;	
		int max_y = AABB->Max.y;	
		int max_x = AABB->Max.x;	

		// Min row and col
		int min_row = floor(min_y / cell_size);
		int min_col = floor(min_x / cell_size);

		// Max row and col
		int max_row = floor(max_y / cell_size);
		int max_col = floor(max_x / cell_size);

		int max_allowed_index = grid->rows * grid->cols;

		for (int i = min_row; i <= max_row; i++)
		{
			if (i >= grid->rows) continue;
			if (i < 0) continue;

			for (int j = min_col; j <= max_col; j++)
			{
				if (j >= grid->cols) continue;
				if (j < 0) continue;

				int index = i * grid->cols + j;
				if (index < 0 || index >= max_allowed_index) continue;

				// Add all entities in this cell into entities vector for return 
				entities.insert(entities.end(), grid->cells[index].entities.begin(), grid->cells[index].entities.end());

				// Then push back this entity
				grid->cells[index].entities.push_back(entity);

				// Keep track of "dirty" cells here
				grid->dirtyCells.push_back(index);
			}
		}

		return entities;
	}

	/* Clears all entity vectors from every cell in the spatial grid */
	// NOTE(John): This does not scale with large levels, because I'm having to clear ALL cells instead of "dirty" ones
	void ClearCells(Grid* grid) 
	{
		if (grid == nullptr) Enjon::Utils::FatalError("SPATIALHASH::CLEARCELLS::Cannot operate on null data.");

		// Loop through dirty cells and clear entities from those
		for (Enjon::uint32 i : grid->dirtyCells)
		{
			grid->cells[i].entities.clear();
		}

		// Clear dirty cells
		grid->dirtyCells.clear();
	}


	/* Finds all neighboring cells to a given entitiy's cell and stores those in a passed in entities vector */
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

	void DrawGrid(Grid* G, EG::SpriteBatch* Batch, EM::Vec2& Position)
	{
		// Get starting cell
		auto StartCell = FindCellCoordinates(G, &Position);
		EM::Vec2 DebugRadius(40);

		// Set radius
		auto R = (int)(StartCell.y - DebugRadius.y);
		auto C = (int)(StartCell.x - DebugRadius.x);

		// Set max row and column
		auto MaxR = R + 2 * DebugRadius.y > G->rows ? G->rows : R + 2 * DebugRadius.y;
		auto MaxC = C + 2 * DebugRadius.x > G->cols ? G->cols : C + 2 * DebugRadius.x;

		// Make sure in bounds
		if (R > G->rows - 1) R = G->rows - 1;
		if (R < 0) R = 0;

		if (C > G->cols - 1) C = G->cols - 1;
		if (C < 0) C = 0;

		// Set width and height
		auto Width = CELL_SIZE * 2.0f;
		auto Height = CELL_SIZE;

		// Get X and Y
		auto X = G->Origin.x;
		auto Y = G->Origin.y;

		// Get row offset
		for (auto i = 0; i < R; i++)
		{
			X -= Width / 2.0f;
			Y += Height / 2.0f;
		}

		// Get column offset
		for (auto i = 0; i < C; i++)
		{
			X += Width / 2.0f; 
			Y += Height / 2.0f;
		}

		// Set current x and y
		auto CurrentX = X;
		auto CurrentY = Y;
	
		// Draw spatial grid
		for (auto r = R; r < MaxR; r++)
		{
			for (auto c = C; c < MaxC; c++)
			{
				EG::ColorRGBA16 Color = EG::SetOpacity(EG::RGBA16_SkyBlue(), 0.4f);
				auto index = r * G->cols + c;
				auto size = G->cells.at(index).entities.size();
				if (size) Color = EG::SetOpacity(EG::RGBA16_Green(), 0.4f);
				Batch->Add(
							EM::Vec4(CurrentX, CurrentY, Width - 5.0f, Height - 5.0f), 
							EM::Vec4(0, 0, 1, 1), 
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png").id,
							Color
						);

				CurrentX += Width / 2.0f;
				CurrentY += Height / 2.0f;
			}

			X -= Width / 2.0f;
			Y += Height / 2.0f;
			CurrentX = X;
			CurrentY = Y;
		}
		
	}
}