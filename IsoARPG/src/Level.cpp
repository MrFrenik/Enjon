#include "Level.h" 
#include "System/Types.h"
#include "Defines.h"

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

	// m_tilesheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall.png"), EM::iVec2(1, 1));
	m_tilesheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledred.png"), EM::iVec2(1, 1));
	// m_wallSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/wall_chunk.png"), EM::iVec2(1, 1));

	// Set debug draw to false
	m_DrawDebugEnabled = true;

	CurrentOverlayIndex = 0;

	// Overlays are clean
	m_OverlaysDirty = false;
	
	// float tilewidth = 512.0f;
	float tilewidth = 200.0f;
	float tileheight = tilewidth / 2.0f;
	float wallWidth = tilewidth;
	float wallHeight = 7.6f * tilewidth;
	unsigned int index = 0;

	m_rows = rows;
	m_cols = cols;

	m_width = (int)(tilewidth / 2.0f) * cols;
	m_height = (int)(tileheight / 2.0f) * rows;

	m_cartesianWidth = tilewidth / 2.0f * cols;
	m_cartesianHeight = tilewidth / 2.0f * rows;

	// float X = x + (cols * tilewidth / 2.0f);
	float X = x;
	float Y = y;
	float currentX = X;
	float currentY = Y; 

	//Grab Iso and Cartesian tile data
	for (int i = 0; i < rows; i++)
	{ 
		for (int j = 0; j < cols; j++)
		{
			//Add coordinates of iso world to tile data
			Tile tile(EM::Vec2(currentX, currentY), EM::Vec2(tilewidth, tileheight), 2.0f, &m_tilesheet, 1);
			m_mapTiles.emplace_back(tile);
			m_GroundTiles.emplace_back(tile);

			// //Add coordinates of 2d world to level data vector
			m_cartesiantiles.emplace_back(
											Tile(
													// Enjon::Math::IsoToCartesian(Enjon::Math::Vec2(currentX + tilewidth / 2.0f, currentY + tileheight)), 
													Enjon::Math::IsoToCartesian(Enjon::Math::Vec2(currentX, currentY)), 
													Enjon::Math::Vec2(tilewidth / 2.0f), 
													2.0f, 
													&m_tilesheet, 
													index
												)
										);

			//Increment currentX and currentY	
			currentX -= (tilewidth / 2.0f);
			currentY += (tileheight / 2.0f);
		}

		//Go up a row
		X += tilewidth / 2.0f;
		Y += tileheight / 2.0f;
		currentX = X;
		currentY = Y;
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

void Level::DrawGroundTiles(Enjon::Graphics::SpriteBatch& Batch, EG::SpriteBatch& Normals)
{
	auto C = EG::RGBA16_White();
	auto DC = 190.0f / 255.0f;
	C.r -= DC;
	C.g -= DC;
	C.b -= DC;
	//Add level information to Batch
	for (Tile& tile : m_GroundTiles)
	{
		Batch.Add(
					EM::Vec4(tile.pos, EM::Vec2(tile.dims.x, tile.dims.y)), 
					tile.Sheet->GetUV(tile.index), 
					tile.Sheet->texture.id, 
					C, 
					0 
				);

		// Normals.Add(
		// 				EM::Vec4(tile.pos, tile.dims), 
		// 				EM::Vec4(0, 0, 1, 1), 
		// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/brickwall_normal.png").id, 
		// 				EG::RGBA16_White(), 
		// 				0
		// 			);
	}
}

void Level::DrawDebugTiles(Enjon::Graphics::SpriteBatch& Batch)
{
	for (Tile& tile : m_GroundTiles)
	{
		Batch.Add(
					EM::Vec4(tile.pos, EM::Vec2(tile.dims.x, tile.dims.y)), 
					EM::Vec4(0, 0, 1, 1), 
					EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png").id, 
					EG::SetOpacity(EG::RGBA16_Red(), 0.5f)
				);
	}

	// for (Tile& tile : m_cartesiantiles)
	// {
	// 	Batch.Add(
	// 				EM::Vec4(tile.pos, EM::Vec2(tile.dims.x, tile.dims.y)), 
	// 				EM::Vec4(0, 0, 1, 1), 
	// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
	// 				EG::SetOpacity(EG::RGBA16_Red(), 0.5f)
	// 			);
	// }
}

void Level::DrawDebugActiveTile(EG::SpriteBatch& Batch, EM::Vec2& Position)
{
	EM::Vec2 TilePostiion;
	EM::Vec2 Dimensions;

	// Find the tile that the position is covering
	auto CartPos = EM::IsoToCartesian(Position);

	// auto x_percentage = 



	auto T = &m_cartesiantiles.at((m_rows - 1) * (m_cols));
	TilePostiion = T->pos;
	Dimensions = T->dims;

	Batch.Add(
				EM::Vec4(TilePostiion, Dimensions), 
				EM::Vec4(0, 0, 1, 1),
				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
				EG::SetOpacity(EG::RGBA16_Green(), 1.0f)
			);

	T = &m_cartesiantiles.at((0) * (m_cols));
	TilePostiion = T->pos;
	Dimensions = T->dims;

	Batch.Add(
				EM::Vec4(TilePostiion, Dimensions), 
				EM::Vec4(0, 0, 1, 1),
				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
				EG::SetOpacity(EG::RGBA16_Green(), 1.0f)
			);

	Batch.Add(
				EM::Vec4(CartPos, Dimensions), 
				EM::Vec4(0, 0, 1, 1),
				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
				EG::SetOpacity(EG::RGBA16_Blue(), 1.0f)
			);

}

void Level::DrawIsoLevel(Enjon::Graphics::SpriteBatch& batch)
{ 
	//Add level information to batch
	for (Tile& tile : m_isotiles)
	{
		batch.Add(EM::Vec4(tile.pos, tile.dims), tile.Sheet->GetUV(tile.index), tile.Sheet->texture.id);
	} 
}

void Level::DrawIsoLevelFront(Enjon::Graphics::SpriteBatch& batch)
{
	for (Tile& tile : m_isoTilesFront)
	{
		batch.Add(EM::Vec4(tile.pos, tile.dims), tile.Sheet->GetUV(tile.index), tile.Sheet->texture.id);
	}
}

void Level::DrawCartesianLevel(Enjon::Graphics::SpriteBatch& Batch)
{ 
	for (Tile& tile : m_cartesiantiles)
	{
		Batch.Add(
					EM::Vec4(tile.pos, tile.dims), 
					EM::Vec4(0, 0, 1, 1), 
					EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
				  	Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA16_Orange(), 0.1f)
				  );
	}
} 

void Level::DrawMap(Enjon::Graphics::SpriteBatch& batch)
{
	for (Tile& tile : m_mapTiles)
	{
		batch.Add(EM::Vec4(tile.pos, tile.dims), tile.Sheet->GetUV(tile.index), 0, Enjon::Graphics::SetOpacity(Enjon::Graphics::RGBA16_Black(), 0.2f), 100.0f);
	}	
}

void Level::DrawTileOverlays(Enjon::Graphics::SpriteBatch& batch)
{
	for (TileOverlay& TO : m_TileOverlays)
	{
		batch.Add(TO.DestRect, EM::Vec4(0, 0, 1, 1), TO.Tex.id, TO.Color);
	}

	// Set to not dirty
	m_OverlaysDirty = false;
}

void Level::AddTileOverlay(Enjon::Graphics::GLTexture Tex, EM::Vec4 DestRect, Enjon::Graphics::ColorRGBA16 Color)
{
	struct TileOverlay TO = TileOverlay{Tex, DestRect, Color};
	if (m_TileOverlays.size() == MAX_TILE_OVERLAY - 1)
	{
		if (CurrentOverlayIndex >= MAX_TILE_OVERLAY) CurrentOverlayIndex = 0;
		m_TileOverlays[CurrentOverlayIndex++] = TO;
	}
	else m_TileOverlays.push_back(TO);
	m_OverlaysDirty = true;
}


void Level::CleanOverlays()
{
	m_OverlaysDirty = false;
}

bool Level::GetOverlaysDirty()
{
	return m_OverlaysDirty;
}



