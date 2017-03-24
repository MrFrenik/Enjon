#ifndef ENJON_SPRITE_SHEET_MANAGER_H
#define ENJON_SPRITE_SHEET_MANAGER_H

#include "Graphics/SpriteSheet.h"
#include "IO/ResourceManager.h"
#include "Math/Maths.h"

#include <unordered_map>

namespace Enjon { namespace SpriteSheetManager {

	void Init();

	/* Add spritesheets to the spritesheetmanager */
	void AddSpriteSheet(std::unordered_map<std::string, SpriteSheet>& M, std::string N, GLTexture T, Math::iVec2 V);

	SpriteSheet* GetSpriteSheet(std::string N);
}}



#endif