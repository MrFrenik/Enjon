#include "Graphics/FontManager.h"

namespace Enjon { namespace Graphics { namespace FontManager {

	std::unordered_map<std::string, Fonts::Font*> Fonts;
	bool Initialized = false;

	/* Checks init status of manager */
	bool IsInit() { return Initialized; }

	/* Init the FontManager */
	void Init()
	{
		AddFont(Fonts, "Bold_12", Fonts::CreateFont("../assets/fonts/TheBoldFont/TheBoldFont.ttf", 12));
		AddFont(Fonts, "Bold_32", Fonts::CreateFont("../assets/fonts/TheBoldFont/TheBoldFont.ttf", 32));
		AddFont(Fonts, "Sofia", Fonts::CreateFont("../assets/fonts/SofiaPro/SofiaPro.otf", 14));
		AddFont(Fonts, "Corbert_12", Fonts::CreateFont("../assets/fonts/CorbertRegular/CorbertRegular.otf", 12));
		AddFont(Fonts, "Corbert_16", Fonts::CreateFont("../assets/fonts/CorbertRegular/CorbertRegular.otf", 16));
		AddFont(Fonts, "Dense", Fonts::CreateFont("../assets/fonts/DenseRegular/DenseRegular.otf", 64));
		AddFont(Fonts, "8Bit", Fonts::CreateFont("../assets/fonts/8BitWonder/8BitWonder.ttf", 8));
		AddFont(Fonts, "8Bit_32", Fonts::CreateFont("../assets/fonts/8BitWonder/8BitWonder.ttf", 32));
		AddFont(Fonts, "TallBold", Fonts::CreateFont("../assets/fonts/TallBolder/TallBolder.ttf", 12));
		AddFont(Fonts, "BebasNeue", Fonts::CreateFont("../assets/fonts/BebasNeue/BebasNeue.otf", 16));
		AddFont(Fonts, "BebasNeue_32", Fonts::CreateFont("../assets/fonts/BebasNeue/BebasNeue.otf", 32));
		AddFont(Fonts, "WeblySleek", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 14));
		AddFont(Fonts, "WeblySleek_32", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 32));
		AddFont(Fonts, "WeblySleek_16", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 16));
		AddFont(Fonts, "WeblySleek_14", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 14));
		AddFont(Fonts, "WeblySleek_12", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 12));
		AddFont(Fonts, "WeblySleek_10", Fonts::CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 10));
		AddFont(Fonts, "Reduction_24", Fonts::CreateFont("../assets/fonts/Reduction/Reduction.ttf", 24));
		AddFont(Fonts, "Reduction_14", Fonts::CreateFont("../assets/fonts/Reduction/Reduction.ttf", 14));
		AddFont(Fonts, "Reduction_10", Fonts::CreateFont("../assets/fonts/Reduction/Reduction.ttf", 10));
		AddFont(Fonts, "VCR_OSD_MONO", Fonts::CreateFont("../assets/fonts/VCR_OSD_MONO/VCR_OSD_MONO.ttf", 14));
		AddFont(Fonts, "Villeray_Semilight_14", Fonts::CreateFont("../assets/fonts/Villeray/Villeray-Semilight.ttf", 14));
		AddFont(Fonts, "Villeray_Semilight_10", Fonts::CreateFont("../assets/fonts/Villeray/Villeray-Semilight.ttf", 10));
		AddFont(Fonts, "Villeray_Bold", Fonts::CreateFont("../assets/fonts/Villeray/Villeray-Bold.ttf", 10));
		AddFont(Fonts, "Arrows7", Fonts::CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 10));
		AddFont(Fonts, "Arrows7_12", Fonts::CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 12));
		AddFont(Fonts, "Arrows7_24", Fonts::CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 24));
		AddFont(Fonts, "Arrows7_32", Fonts::CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 32));
		AddFont(Fonts, "CutOut", Fonts::CreateFont("../assets/fonts/CutOut/CutOut.ttf", 12));

		// Set to being initialized
		Initialized = true;
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
		else 
		{
			std::cout << FontName << " not found." << std::endl;
			return Fonts["WeblySleek"];
		}
	}

	// Delete all animations and clear any memory used
	void DeletFonts()
	{

	}
}}}





