#include "Graphics/SpriteSheetManager.h"
#include <iostream>

namespace Enjon { namespace SpriteSheetManager {

	std::unordered_map<std::string, SpriteSheet> Sheets;

	void Init()
	{
		AddSpriteSheet(Sheets, std::string("PlayerSheet"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtestframessplitsmall.png"), Enjon::iVec2(6, 24));
		AddSpriteSheet(Sheets, std::string("PlayerSheet2"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/axe.png"), Enjon::iVec2(8, 2));
		AddSpriteSheet(Sheets, std::string("Orb"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Orb2"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb4.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Wall"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/wall_chunk.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Wall"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/wall_chunk.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Box"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("BoxDebris"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_debris.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("BoxSheet"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_sheet.png"), Enjon::iVec2(2, 1));
		AddSpriteSheet(Sheets, std::string("VerticleBar"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/verticlebar.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField0"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField1"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield2.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField2"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield3.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField3"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield4.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Dude"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/dude.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Enemy"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/enemy.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Beast"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast2.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("2g"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2g_transparent.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Tile"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Gib"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/gib.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("DiagonalTile"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/diagonal_tile.png"), Enjon::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("DiagonalTileDown"), Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/diagonal_tile_down.png"), Enjon::iVec2(1, 1));
	}

	/* Add spritesheets to the spritesheetmanager */
	void AddSpriteSheet(std::unordered_map<std::string, SpriteSheet>& M, std::string N, GLTexture T, Enjon::iVec2 V)
	{
		// Create SpriteSheet	
		SpriteSheet S;

		// Init SpriteSheet
		S.Init(T, V);

		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		M[N] = S;
	}

	/* Get specified animation with given name */
	SpriteSheet* GetSpriteSheet(std::string N)
	{
		auto it = Sheets.find(N);
		if (it != Sheets.end()) return &it->second;
		else return nullptr;
		std::cout << N << "not found!" << std::endl;
	}

}}