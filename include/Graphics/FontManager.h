#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include "Graphics/Font.h"
#include "Utils/Errors.h"

#include <unordered_map>

namespace Enjon { namespace Graphics { namespace FontManager {

	/* Checks init status of manager */
	bool IsInit();

	/* Init the FontManager */
	void Init();

	/* Add font to FontManager */
	void AddFont(std::unordered_map<std::string, Fonts::Font*>& M, std::string N, Fonts::Font* F);

	// Get specificed animation with given name
	Fonts::Font* GetFont(std::string FontName);

	// Delete all animations and clear any memory used
	void DeletFonts();
}}}

#endif




