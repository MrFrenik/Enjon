#include "Graphics/FontManager.h"

namespace Enjon { namespace Graphics { namespace FontManager {

	std::unordered_map<std::string, Fonts::Font*> Fonts;

	/* Init the FontManager */
	void Init()
	{
		AddFont(Fonts, "Bold", Fonts::CreateFont("../assets/fonts/TheBoldFont/TheBoldFont.ttf", 32));
	}

	/* Add font to FontManager */
	void AddFont(std::unordered_map<std::string, Fonts::Font*>& M, std::string N, Fonts::Font* F)
	{
		// Will overwrite pre-existing font if there!
		Fonts[N] = F;
	}

	// Get specificed animation with given name
	Fonts::Font* GetFont(std::string FontName)
	{
		auto it = Fonts.find(FontName);
		if (it != Fonts.end()) return it->second;
		else return Fonts["Bold"];
	}

	// Delete all animations and clear any memory used
	void DeletFonts()
	{

	}
}}}





