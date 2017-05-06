#include "Graphics/FontManager.h"

namespace Enjon { namespace FontManager {

	std::unordered_map<std::string, Font*> Fonts;
	bool Initialized = false;

	/* Checks init status of manager */
	bool IsInit() { return Initialized; }

	/* Init the FontManager */
	void Init()
	{
		AddFont(Fonts, "Bold_12", CreateFont("../assets/fonts/TheBoldFont/TheBoldFont.ttf", 12));
		AddFont(Fonts, "Bold_32", CreateFont("../assets/fonts/TheBoldFont/TheBoldFont.ttf", 32));
		AddFont(Fonts, "Sofia_64", CreateFont("../assets/fonts/SofiaPro/SofiaPro.otf", 64));
		AddFont(Fonts, "Sofia_14", CreateFont("../assets/fonts/SofiaPro/SofiaPro.otf", 14));
		AddFont(Fonts, "Corbert_12", CreateFont("../assets/fonts/CorbertRegular/CorbertRegular.otf", 12));
		AddFont(Fonts, "Corbert_16", CreateFont("../assets/fonts/CorbertRegular/CorbertRegular.otf", 16));
		AddFont(Fonts, "Dense", CreateFont("../assets/fonts/DenseRegular/DenseRegular.otf", 64));
		AddFont(Fonts, "8Bit", CreateFont("../assets/fonts/8BitWonder/8BitWonder.ttf", 8));
		AddFont(Fonts, "8Bit_32", CreateFont("../assets/fonts/8BitWonder/8BitWonder.ttf", 32));
		AddFont(Fonts, "TallBold", CreateFont("../assets/fonts/TallBolder/TallBolder.ttf", 12));
		AddFont(Fonts, "BebasNeue", CreateFont("../assets/fonts/BebasNeue/BebasNeue.otf", 16));
		AddFont(Fonts, "BebasNeue_72", CreateFont("../assets/fonts/BebasNeue/BebasNeue.otf", 72));
		AddFont(Fonts, "BebasNeue_32", CreateFont("../assets/fonts/BebasNeue/BebasNeue.otf", 32));
		AddFont(Fonts, "WeblySleek_128", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 128));
		AddFont(Fonts, "WeblySleek_64", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 32));
		AddFont(Fonts, "WeblySleek_32", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 32));
		AddFont(Fonts, "WeblySleek_16", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 16));
		AddFont(Fonts, "WeblySleek_14", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 14));
		AddFont(Fonts, "WeblySleek_12", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 12));
		AddFont(Fonts, "WeblySleek_10", CreateFont("../assets/fonts/WeblySleek/weblysleekuisb.ttf", 10));
		AddFont(Fonts, "Reduction_24", CreateFont("../assets/fonts/Reduction/Reduction.ttf", 24));
		AddFont(Fonts, "Reduction_14", CreateFont("../assets/fonts/Reduction/Reduction.ttf", 14));
		AddFont(Fonts, "Reduction_10", CreateFont("../assets/fonts/Reduction/Reduction.ttf", 10));
		AddFont(Fonts, "VCR_OSD_MONO", CreateFont("../assets/fonts/VCR_OSD_MONO/VCR_OSD_MONO.ttf", 14));
		AddFont(Fonts, "Villeray_Semilight_14", CreateFont("../assets/fonts/Villeray/Villeray-Semilight.ttf", 14));
		AddFont(Fonts, "Villeray_Semilight_10", CreateFont("../assets/fonts/Villeray/Villeray-Semilight.ttf", 10));
		AddFont(Fonts, "Villeray_Bold_64", CreateFont("../assets/fonts/Villeray/Villeray-Bold.ttf", 64));
		AddFont(Fonts, "Villeray_Bold_10", CreateFont("../assets/fonts/Villeray/Villeray-Bold.ttf", 10));
		AddFont(Fonts, "Arrows7", CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 10));
		AddFont(Fonts, "Arrows7_12", CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 12));
		AddFont(Fonts, "Arrows7_24", CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 24));
		AddFont(Fonts, "Arrows7_32", CreateFont("../assets/fonts/Arrows7/Arrows7.ttf", 32));
		AddFont(Fonts, "CutOut", CreateFont("../assets/fonts/CutOut/CutOut.ttf", 12));
		AddFont(Fonts, "SilomBol_64", CreateFont("../assets/fonts/SilomBol/SilomBol.ttf", 64));
		AddFont(Fonts, "SilomBol_14", CreateFont("../assets/fonts/SilomBol/SilomBol.ttf", 14));
		AddFont(Fonts, "MisterPixelRegular_14", CreateFont("../assets/fonts/MisterPixel/MisterPixelRegular.ttf", 14));

		// Set to being initialized
		Initialized = true;
	}

	/* Add font to FontManager */
	void AddFont(std::unordered_map<std::string, Font*>& M, std::string N, Font* F)
	{
		// Will overwrite pre-existing font if there!
		Fonts[N] = F;
	}

	// Get specificed animation with given name
	Font* GetFont(std::string FontName)
	{
		auto it = Fonts.find(FontName);
		if (it != Fonts.end()) return it->second;
		else 
		{
			std::cout << FontName << " not found." << std::endl;
			return Fonts["WeblySleek_14"];
		}
	}

	// Delete all animations and clear any memory used
	void DeletFonts()
	{

	}
}}





