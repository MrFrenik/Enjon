#include "Level.h" 

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

void Level::Init(float x, float y, int row, int col)
{
	// TODO(John): Consider loading level data from file
	// TODO(John): Levels will be generated dynamically and randomly seeded as well
	// TODO(John): Bound entities within the level 

	m_tilesheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/stone.png"), Enjon::Math::iVec2(3, 1));
	
	//Push tiles back into level data 
	float currentX = x;
	float currentY = y; 
	float tilewidth = 64.0f;
	float tileheight = tilewidth / 2.0f;
	int checker = 0;
	unsigned int index = 0;

	m_width = (int)tilewidth * col / 2.0f;
	m_height = (int)tileheight * row / 2.0f;

	//Grab Iso and Cartesian tile data
	for (int i = 0; i < row; i++)
	{ 
		for (int j = 0; j < col; j++)
		{
			checker++;

			//Add coordinates of iso world to tile data
			index = Enjon::Random::Roll(0, 3);
			if (index == 0 || index == 1) index = Enjon::Random::Roll(0, 3);
			if (index == 0) index = Enjon::Random::Roll(0, 3);
			m_isotiles.emplace_back(Tile(Enjon::Math::Vec2(currentX, currentY), Enjon::Math::Vec2(tilewidth, tileheight), index));

			//Add coordinates of 2d world to level data vector
			m_cartesiantiles.emplace_back(Tile(Enjon::Math::IsoToCartesian(Enjon::Math::Vec2(currentX, currentY)), 
											Enjon::Math::Vec2(tilewidth / 2.0f), index));
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

}

void Level::DrawIsoLevel(Enjon::Graphics::SpriteBatch& batch)
{ 
	//Add level information to batch
	for (Tile& tile : m_isotiles)
	{
		//batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), Enjon::Math::Vec4(0, 0, 1, 1), tile.texture.id);
		batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), m_tilesheet.GetUV(tile.index), m_tilesheet.texture.id);
	} 
}

void Level::DrawCartesianLevel(Enjon::Graphics::SpriteBatch& batch)
{ 
	for (Tile& tile : m_cartesiantiles)
	{
		//batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), Enjon::Math::Vec4(0, 0, 1, 1), tile.texture.id, Enjon::Graphics::ColorRGBA8::White().SetOpacity(0.5f));
		batch.Add(Enjon::Math::Vec4(tile.pos, tile.dims), m_tilesheet.GetUV(tile.index), m_tilesheet.texture.id, 
				  Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA8_White(), 0.5f));
	}
} 
