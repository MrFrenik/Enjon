#include "Graphics/SpriteSheetManager.h"
#include <iostream>

namespace Enjon { namespace Graphics { namespace SpriteSheetManager {

	std::unordered_map<std::string, SpriteSheet> Sheets;

	void Init()
	{
		AddSpriteSheet(Sheets, std::string("PlayerSheet"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtestframessplitsmall.png"), Enjon::Math::iVec2(6, 24));
		AddSpriteSheet(Sheets, std::string("PlayerSheet2"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/axe.png"), Enjon::Math::iVec2(8, 2));
		AddSpriteSheet(Sheets, std::string("Orb"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Orb2"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb4.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Wall"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/wall_chunk.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Wall"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/wall_chunk.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Box"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("BoxDebris"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_debris.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("BoxSheet"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_sheet.png"), Enjon::Math::iVec2(2, 1));
		AddSpriteSheet(Sheets, std::string("VerticleBar"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/verticlebar.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField0"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField1"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield2.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField2"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield3.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("ForceField3"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/forcefield4.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Dude"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/dude.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Enemy"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/enemy.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Beast"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/beast2.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("2g"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/2g_transparent.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Tile"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/tiletestfilledwhite.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("Gib"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/gib.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("DiagonalTile"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/diagonal_tile.png"), Enjon::Math::iVec2(1, 1));
		AddSpriteSheet(Sheets, std::string("DiagonalTileDown"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/diagonal_tile_down.png"), Enjon::Math::iVec2(1, 1));
	}

	/* Add spritesheets to the spritesheetmanager */
	void AddSpriteSheet(std::unordered_map<std::string, SpriteSheet>& M, std::string N, GLTexture T, Math::iVec2 V)
	{
		// Create SpriteSheet	
		Graphics::SpriteSheet S;

		// Init SpriteSheet
		S.Init(T, V);

		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		M[N] = S;
	}

	/* Get specificed animation with given name */
	SpriteSheet* GetSpriteSheet(std::string N)
	{
		auto it = Sheets.find(N);
		if (it != Sheets.end()) return &it->second;
		else return nullptr;
		std::cout << N << "not found!" << std::endl;
	}

}}}