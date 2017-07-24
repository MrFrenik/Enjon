#include "AI/SpatialHash.h"
#include <stdio.h>

#include <Graphics/SpriteBatch.h>
#include <Graphics/Color.h>
#include <IO/ResourceManager.h>

namespace SpatialHash {

	/* Inits spatial grid based on width, height of level and given cell size */ 
	void Init(Grid* G, int width, int height, int cell_size)
	{
		if (G == nullptr) Enjon::Utils::FatalError("SPATIALHASH::INIT::Cannot operate on null data.");

		G->CellSize = cell_size;

		G->rows = ceil((height) / cell_size);
		G->cols = ceil((width) / cell_size);

		// Allocate correct memory size for cells
		G->cells.resize(G->rows * G->cols);

		// Set each cell's obstruction value to 0.0f
		for (auto& c : G->cells)
		{
			c.ObstructionValue = 0.0f;
		}

		// Set origin of G
		// This needs to be passed in by world that creates it
		G->Origin = EM::Vec2(0.0f);
	}

	/* Finds particular cell that a given entity belongs to based on its position */
	EM::Vec2 FindGridCoordinates(Grid* G, V2& Position)
	{
		int posX = Position.x;
		int posY = Position.y; 

		// Get indicies for row and column
		int row = floor(posY / G->CellSize);
		int col = floor(posX / G->CellSize);

		// Bound row and column
		if (row > G->rows - 1) row = G->rows - 1;
		if (row < 0) row = 0;
		if (col > G->cols - 1) col = G->cols - 1;
		if (col < 0) col = 0;


		return EM::Vec2(col, row);
	}

	EM::Vec2 FindGridCoordinatesFromIndex(Grid* G, Enjon::uint32 Index)
	{
		// Calculate row
		auto R = floor(Index / G->cols);

		// Calculate column
		auto C = Index - (R * G->cols);

		return EM::Vec2(C, R);
	}

	std::vector<Enjon::uint32> GetEntitiesFromCells(Grid* G, EM::Vec4& Cells)
	{
		std::vector<Enjon::uint32> Entities;

		auto Border = 4;

		auto MinCol = Cells.x;
		auto MinRow = Cells.y;
		auto MaxCol = Cells.z;
		auto MaxRow = Cells.w;

		auto max_allowed_index = G->rows * G->cols;

		for (auto i = MinRow - Border; i <= MaxRow + Border; i++)
		{
			for (int j = MinCol - Border; j <= MaxCol + Border; j++)
			{
				auto index = i * G->cols + j;
				if (index < 0 || index >= max_allowed_index) continue;

				// Add all entities in this cell into entities vector for return 
				Entities.insert(Entities.end(), G->cells[index].entities.begin(), G->cells[index].entities.end());
			}
		}

		return Entities;
	}

	void FindCells(Grid* G, Enjon::uint32 Entity, const EP::AABB* AABB, EM::Vec4* CellRange, float ObstructionValue)
	{
		// AABB elements
		int min_y = AABB->Min.y;	
		int min_x = AABB->Min.x;	
		int max_y = AABB->Max.y;	
		int max_x = AABB->Max.x;	

		// Min row and col
		int min_row = floor(min_y / G->CellSize);
		int min_col = floor(min_x / G->CellSize);

		// Max row and col
		int max_row = floor(max_y / G->CellSize);
		int max_col = floor(max_x / G->CellSize);


		for (auto i = CellRange->y; i < CellRange->w; i++)
		{
			for (auto j = CellRange->x; j < CellRange->z; j++)
			{
				auto index = i * G->cols + j;
				// if (G->cells[index].ObstructionValue > 0) G->cells[index].ObstructionValue -= ObstructionValue;
			}
		}

		// Set cell range
		CellRange->x = min_col;
		CellRange->y = min_row;
		CellRange->z = max_col;
		CellRange->w = max_row;

		int max_allowed_index = G->rows * G->cols;

		for (int i = min_row; i <= max_row; i++)
		{
			if (i >= G->rows) continue;
			if (i < 0) continue;

			for (int j = min_col; j <= max_col; j++)
			{
				if (j >= G->cols) continue;
				if (j < 0) continue;

				int index = i * G->cols + j;
				if (index < 0 || index >= max_allowed_index) continue;

				// Then push back this entity
				G->cells[index].entities.push_back(Entity);

				// if (G->cells[index].ObstructionValue < 1.0f) G->cells[index].ObstructionValue += ObstructionValue;

				// Keep track of "dirty" cells here
				G->dirtyCells.push_back(index);
			}
		}
	}

	/*
	void FindCells(Grid* G, Enjon::uint32 Entity, const EP::AABB* AABB, EM::Vec4* CellRange, float ObstructionValue)
	{
		// AABB elements
		int min_y = AABB->Min.y;	
		int min_x = AABB->Min.x;	
		int max_y = AABB->Max.y;	
		int max_x = AABB->Max.x;	

		// Min row and col
		int min_row = floor(min_y / G->CellSize);
		int min_col = floor(min_x / G->CellSize);

		// Max row and col
		int max_row = floor(max_y / G->CellSize);
		int max_col = floor(max_x / G->CellSize);

		auto Border = 0;
		auto AdditiveValue = 0.7f;

		auto MinRow = CellRange->y - Border > 0 ? CellRange->y - Border : 0;
		auto MinCol = CellRange->x - Border > 0 ? CellRange->x - Border : 0;
		auto MaxRow = CellRange->w + Border < G->rows ? CellRange->w + Border : G->rows;
		auto MaxCol = CellRange->z + Border < G->cols ? CellRange->z + Border : G->cols;

		for (auto i = MinRow; i < MaxRow; i++)
		{
			for (auto j = MinCol; j < MaxCol; j++)
			{
				auto index = i * G->cols + j;
				if (i < CellRange->y || i > CellRange->w || j < CellRange->x || j > CellRange->z)
				{
					if (G->cells[index].ObstructionValue > 0) G->cells[index].ObstructionValue -= ObstructionValue;
					G->cells[index].ObstructionValue += AdditiveValue;

				}
				else
				{
					if (G->cells[index].ObstructionValue > 0) G->cells[index].ObstructionValue -= ObstructionValue;
				}
			}
		}

		// Set cell range
		CellRange->x = min_col;
		CellRange->y = min_row;
		CellRange->z = max_col;
		CellRange->w = max_row;

		MinRow = CellRange->y - Border > 0 ? CellRange->y - Border : 0;
		MinCol = CellRange->x - Border > 0 ? CellRange->x - Border : 0;
		MaxRow = CellRange->w + Border < G->rows ? CellRange->w + Border : G->rows;
		MaxCol = CellRange->z + Border < G->cols ? CellRange->z + Border : G->cols;

		int max_allowed_index = G->rows * G->cols;

		for (int i = MinRow; i < MaxRow; i++)
		{
			if (i >= G->rows) continue;
			if (i < 0) continue;

			for (int j = MinCol; j <= MaxCol; j++)
			{
				if (j >= G->cols) continue;
				if (j < 0) continue;

				int index = i * G->cols + j;

				// Then push back this entity
				G->cells[index].entities.push_back(Entity);

				// Keep track of "dirty" cells here
				G->dirtyCells.push_back(index);

				if (i < CellRange->y || i > CellRange->w || j < CellRange->x || j > CellRange->z)
				{
					if (G->cells[index].ObstructionValue < 1.0f) G->cells[index].ObstructionValue += ObstructionValue;
					G->cells[index].ObstructionValue -= AdditiveValue;

				}
				else
				{
					if (G->cells[index].ObstructionValue < 1.0f) G->cells[index].ObstructionValue += ObstructionValue;
				}
			}
		}
	}
	*/

	/* Overloaded function that finds particular cell that a given entity belongs to based on its AABB (preferred method) */
	std::vector<Enjon::uint32> FindCell(Grid* G, Enjon::uint32 entity, const EP::AABB* AABB)
	{
		std::vector<Enjon::uint32> entities;

		// AABB elements
		int min_y = AABB->Min.y;	
		int min_x = AABB->Min.x;	
		int max_y = AABB->Max.y;	
		int max_x = AABB->Max.x;	

		// Min row and col
		int min_row = floor(min_y / G->CellSize);
		int min_col = floor(min_x / G->CellSize);

		// Max row and col
		int max_row = floor(max_y / G->CellSize);
		int max_col = floor(max_x / G->CellSize);

		int max_allowed_index = G->rows * G->cols;

		for (int i = min_row; i <= max_row; i++)
		{
			if (i >= G->rows) continue;
			if (i < 0) continue;

			for (int j = min_col; j <= max_col; j++)
			{
				if (j >= G->cols) continue;
				if (j < 0) continue;

				int index = i * G->cols + j;
				if (index < 0 || index >= max_allowed_index) continue;

				// Add all entities in this cell into entities vector for return 
				entities.insert(entities.end(), G->cells[index].entities.begin(), G->cells[index].entities.end());

				// Then push back this entity
				G->cells[index].entities.push_back(entity);

				// Keep track of "dirty" cells here
				G->dirtyCells.push_back(index);
			}
		}

		return entities;
	}

	/* Clears all entity vectors from every cell in the spatial grid */
	// NOTE(John): This does not scale with large levels, because I'm having to clear ALL cells instead of "dirty" ones
	void ClearCells(Grid* G) 
	{
		if (G == nullptr) Enjon::Utils::FatalError("SPATIALHASH::CLEARCELLS::Cannot operate on null data.");

		// Loop through dirty cells and clear entities from those
		for (Enjon::uint32 i : G->dirtyCells)
		{
			G->cells[i].entities.clear();

			// Reset OB value
			// G->cells[i].ObstructionValue = 0.0f;
		}

		// Clear dirty cells
		G->dirtyCells.clear();
	}


	/* Finds all neighboring cells to a given entitiy's cell and stores those in a passed in entities vector */
	// TODO(John): Create a pair checking function to maintain and update pairs of entities that have already been checked with one another
	// NOTE(John): As of now, this is incredibly too slow to work...
	void GetNeighborCells(Grid* G, int index, std::vector<Enjon::uint32>* Entities)
	{
		// Grab all neighbor cell indexes	
		int Top = index - G->cols; 
		int TopLeft = Top - 1; 
		int TopRight = Top + 1; 
		int Left = index - 1;
		int Right = index + 1;
		int BottomLeft = index + G->cols - 1;
		int Bottom = index + G->cols;
		int BottomRight = index + G->cols + 1;
		int max = G->rows * G->cols;

		// If valid, append to entities
		if (Top >= 0)		   if (!G->cells[Top].entities.empty())  			Entities->insert(Entities->end(), G->cells[Top].entities.begin(), G->cells[Top].entities.end()); 
		if (Bottom < max)	   if (!G->cells[Bottom].entities.empty()) 			Entities->insert(Entities->end(), G->cells[Bottom].entities.begin(), G->cells[Bottom].entities.end());
		if (TopLeft >= 0)	   if (!G->cells[TopLeft].entities.empty()) 			Entities->insert(Entities->end(), G->cells[TopLeft].entities.begin(), G->cells[TopLeft].entities.end());
		if (TopRight >= 0)	   if (!G->cells[TopRight].entities.empty()) 		Entities->insert(Entities->end(), G->cells[TopRight].entities.begin(), G->cells[TopRight].entities.end());
		if (Left >= 0)		   if (!G->cells[Left].entities.empty()) 			Entities->insert(Entities->end(), G->cells[Left].entities.begin(), G->cells[Left].entities.end());
		if (Right < max)	   if (!G->cells[Right].entities.empty()) 			Entities->insert(Entities->end(), G->cells[Right].entities.begin(), G->cells[Right].entities.end());
		if (BottomLeft < max)  if (!G->cells[BottomLeft].entities.empty()) 		Entities->insert(Entities->end(), G->cells[BottomLeft].entities.begin(), G->cells[BottomLeft].entities.end());
		if (BottomRight < max) if (!G->cells[BottomRight].entities.empty()) 		Entities->insert(Entities->end(), G->cells[BottomRight].entities.begin(), G->cells[BottomRight].entities.end());
	}

	Enjon::uint32 GetGridIndexFromCoordinates(Grid* G, EM::Vec2& Coordinates)
	{
		return Coordinates.y * G->cols + Coordinates.x;
	}

	void DrawGrid(Grid* G, EG::SpriteBatch* Batch, EM::Vec2& Position)
	{
		// Get starting cell
		auto StartCell = FindGridCoordinates(G, Position);
		EM::Vec2 DebugRadius(40);

		// Set radius
		auto R = (int)(StartCell.y - DebugRadius.y);
		auto C = (int)(StartCell.x - DebugRadius.x);

		// Set max row and column
		auto MaxR = R + 2 * DebugRadius.y > G->rows ? G->rows : R + 2 * DebugRadius.y;
		auto MaxC = C + 2 * DebugRadius.x > G->cols ? G->cols : C + 2 * DebugRadius.x;

		// Make sure in bounds
		if (R > G->rows) R = G->rows;
		if (R < 0) R = 0;

		if (C > G->cols) C = G->cols;
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
				auto index = r * G->cols + c;
				auto size = G->cells.at(index).entities.size();
				auto OV = G->cells.at(index).ObstructionValue; 

				Enjon::ColorRGBA16 Color = Enjon::SetOpacity(Enjon::RGBA16_SkyBlue(), 0.3f);
				if (size || OV != 0.0f) Color = Enjon::SetOpacity(Enjon::RGBA16_Green(), OV);
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

	EM::Vec4 GetCellDimensions(Grid* G, EM::Vec2& Cell)
	{
		EM::Vec4 CellDimensions;

		// Get row and column
		auto R = (int)(Cell.y);
		auto C = (int)(Cell.x);

		// Set width and height
		auto Width = CELL_SIZE * 2.0f;
		auto Height = CELL_SIZE;

		// Get origin X and Y
		CellDimensions.x = G->Origin.x;
		CellDimensions.y = G->Origin.y;

		// Set dimensions
		CellDimensions.z = Width;
		CellDimensions.w = Height;

		// Calculate final X
		CellDimensions.x -= (Width / 2.0f) * R;
		CellDimensions.x += (Width / 2.0f) * C;

		// Calculate final Y
		CellDimensions.y += (Height / 2.0f) * R;
		CellDimensions.y += (Height / 2.0f) * C;

		return CellDimensions;
	}

	void DrawActiveCell(Grid* G, EG::SpriteBatch* Batch, EM::Vec2& Position)
	{
		// Get cell of position
		auto StartCell = FindGridCoordinates(G, Position);

		// Get dimensions of cell
		auto CellDimensions = GetCellDimensions(G, StartCell);
	
		// Draw actived tile
		Enjon::ColorRGBA16 Color = Enjon::SetOpacity(Enjon::RGBA16_Orange(), 1.0f);
		auto index = StartCell.y * G->cols + StartCell.x;
		Batch->Add(
					EM::Vec4(CellDimensions.x, CellDimensions.y, CellDimensions.z - 5.0f, CellDimensions.w - 5.0f), 
					EM::Vec4(0, 0, 1, 1), 
					EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png").id,
					Color
				);
	}
}

















