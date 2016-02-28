#include "Graphics/SpriteSheetManager.h"

namespace Enjon { namespace Graphics { namespace SpriteSheetManager {

	std::unordered_map<std::string, SpriteSheet> Sheets;

	void Init()
	{
		AddSpriteSheet(Sheets, std::string("PlayerSheet"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/pixelanimtestframessplitsmall.png"), Enjon::Math::iVec2(6, 24));
		AddSpriteSheet(Sheets, std::string("PlayerSheet2"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/axe.png"), Enjon::Math::iVec2(8, 2));
		AddSpriteSheet(Sheets, std::string("Orb"), Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));
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
		printf("Not found!");
	}

}}}