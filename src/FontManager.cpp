#include "Graphics/FontManager.h"

namespace Enjon { namespace Graphics { namespace FontManager {

	std::unordered_map<std::string, Fonts::Font*> Fonts;

	/* Init the FontManager */
	void Init()
	{
		AddFont(Fonts, "Bold", Fonts::CreateFont("../assets/fonts/TheBoldFont/TheBoldFont.ttf", 32));
		AddFont(Fonts, "Sofia", Fonts::CreateFont("../assets/fonts/SofiaPro/SofiaPro.otf", 32));
		AddFont(Fonts, "Corbert", Fonts::CreateFont("../assets/fonts/CorbertRegular/CorbertRegular.otf", 32));
		AddFont(Fonts, "Dense", Fonts::CreateFont("../assets/fonts/DenseRegular/DenseRegular.otf", 64));
		AddFont(Fonts, "8Bit", Fonts::CreateFont("../assets/fonts/8BitWonder/8BitWonder.ttf", 24));
		AddFont(Fonts, "TallBold", Fonts::CreateFont("../assets/fonts/TallBolder/TallBolder.ttf", 32));
		AddFont(Fonts, "BebasNeue", Fonts::CreateFont("../assets/fonts/BebasNeue/BebasNeue.otf", 16));
		AddFont(Fonts, "WeblySleek", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 14));
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





