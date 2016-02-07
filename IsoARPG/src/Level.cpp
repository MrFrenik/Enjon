#include "Level.h" 
#include "System/Types.h"

#include <IO/ResourceManager.h>

Level::~Level()
{ 
	//Clear tile data
	//for (Tile* tile : m_tiles)
	//{
	//	//delete tile if it exists
	//	if (tile) delete tile;
	//	tile = nullptr;
	//}
}

void Level::Init(float x, float y, int rows, int cols)
{
	// TODO(John): Consider loading level data from file
	// TODO(John): Levels will be generated dynamically and randomly seeded as well
	// TODO(John): Bound entities within the level 

	m_tilesheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/stone.png"), Enjon::Math::iVec2(3, 1));
	m_wallSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/wall_chunk.png"), Enjon::Math::iVec2(1, 1));

	
	//Push tiles back into level data 
	float currentX = x;
	float currentY = y; 
	float tilewidth = 256.0f;
	float tileheight = tilewidth / 2.0f;
	float wallWidth = tilewidth;
	float wallHeight = 7.6f * tilewidth;
	int checker = 0;
	unsigned int index = 0;

	m_width = (int)tilewidth * cols / 2.0f;
	m_height = (int)tileheight * rows / 2.0f;

	//Grab Iso and Cartesian tile data
	for (int i = 0; i < rows; i++)
	{ 
		for (int j = 0; j < cols; j++)
		{
			checker++;

			// Add a wall at edges
			if (IsBorder(i, j, rows, cols))
			{
				// Need to find out where to place these, whether at front or back of rendering process
				if (IsFront(i, j, rows, cols))
				{
					// Add a wall to front render iso world
					m_isoTilesFront.emplace_back(Tile(Enjon::Math::Vec2(currentX, currentY), Enjon::Math::Vec2(wallWidth, wallHeight), currentY, &m_wallSheet, index));
					
					m_isotiles.emplace_back(Tile(Enjon::Math::Vec2(currentX, currentY), Enjon::Math::Vec2(wallWidth, wallHeight), currentY, &m_wallSheet, index));
				}
				else
				{
					// Add a wall to iso world
					m_isotiles.emplace_back(Tile(Enjon::Math::Vec2(currentX, currentY), Enjon::Math::Vec2(wallWidth, wallHeight), currentY, &m_wallSheet, index));
					// Add coordinates of 2D world to cartesian data
				}

				//Add coordinates of 2d world to level data vector
				m_cartesiantiles.emplace_back(Tile(Enjon::Math::IsoToCartesian(Enjon::Math::Vec2(currentX, currentY)), 
												Enjon::Math::Vec2(tilewidth / 2.0f), 2.0f, &m_tilesheet, index));

			}

			// Add tile
			 else
			 {
				//Add coordinates of iso world to tile data
				index = Enjon::Random::Roll(0, 3);
				if (index == 0 || index == 1) index = Enjon::Random::Roll(0, 3);
				if (index == 0) index = Enjon::Random::Roll(0, 3);
				Tile tile(Enjon::Math::Vec2(currentX, currentY), Enjon::Math::Vec2(tilewidth, tileheight), 2.0f, &m_tilesheet, index);
				m_isotiles.emplace_back(tile);
				m_mapTiles.emplace_back(tile);

				//Add coordinates of 2d world to level data vector
				m_cartesiantiles.emplace_back(Tile(Enjon::Math::IsoToCartesian(Enjon::Math::Vec2(currentX, currentY)), 
												Enjon::Math::Vec2(tilewidth / 2.0f), 2.0f, &m_tilesheet, index));
			 }

			//Increment currentX and currentY	
			currentX += (tilewidth / 2.0f);
			currentY -= (tileheight / 2.0f);

		}

		//Go down a row
		x -= tilewidth / 2.0f;
		y -= tileheight / 2.0f;
		currentX = x;
		currentY = y;
		checker++;
	} 

	// Resize glyph pointers to glyph vector size
	m_isoTilePointers.resize(m_isotiles.size()); 

	// Set up glyph pointers to point to corresponding glyphs
	for (uint32_t i = 0; i < m_isotiles.size(); i++)
	{	
		m_isoTilePointers[i] = &m_isotiles[i];
	}

}

bool Level::IsFront(int i, int j, int rows, int cols)
{
	if (i == rows - 1) return true;
	if (j == cols - 1) return true;
	return false;
}

bool Level::IsBorder(int i, int j, int rows, int cols)
{
	if (i == 0 || i == rows - 1) return true;
	if (j == 0 || j == cols - 1) return true;
	return false;
}

void Level::DrawIsoLevel(Enjon::Graphics::SpriteBatch& batch)
{ 
	//Add level information to batch
	for (Tile& tile : m_isotiles)
	{
		//batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), Enjon::Math::Vec4(0, 0, 1, 1), tile.texture.id);
		batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), tile.Sheet->GetUV(tile.index), tile.Sheet->texture.id);
	} 
}

void Level::DrawIsoLevelFront(Enjon::Graphics::SpriteBatch& batch)
{
	for (Tile& tile : m_isoTilesFront)
	{
		//batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), Enjon::Math::Vec4(0, 0, 1, 1), tile.texture.id);
		batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), tile.Sheet->GetUV(tile.index), tile.Sheet->texture.id);
	}
}

void Level::DrawCartesianLevel(Enjon::Graphics::SpriteBatch& batch)
{ 
	for (Tile& tile : m_cartesiantiles)
	{
		// batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), Enjon::Math::Vec4(0, 0, 1, 1), tile.Sheet->texture.id, Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA8_White(), 0.25f));
		batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), Enjon::Math::Vec4(0, 0, 1, 1), 0, 
				  Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA8_Orange(), 0.1f));
	}
} 

void Level::DrawMap(Enjon::Graphics::SpriteBatch& batch)
{
	for (Tile& tile : m_mapTiles)
	{
		batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), tile.Sheet->GetUV(tile.index), 0, Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA8_Black(), 0.2f), 100.0f);
	}	
}
