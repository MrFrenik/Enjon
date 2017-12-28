#include "Graphics/FontManager.h" 
#include "Engine.h"

namespace Enjon { namespace FontManager {

	std::unordered_map<std::string, Font*> Fonts;
	bool Initialized = false;

	/* Checks init status of manager */
	bool IsInit() { return Initialized; }

	/* Init the FontManager */
	void Init()
	{
		Enjon::String rp = Enjon::Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( );
		Enjon::String fp = rp + "/Fonts/";

		AddFont(Fonts, "Bold_12", CreateFont(fp + "TheBoldFont/TheBoldFont.ttf", 12));
		AddFont(Fonts, "Bold_32", CreateFont(fp + "TheBoldFont/TheBoldFont.ttf", 32));
		AddFont(Fonts, "Sofia_64", CreateFont(fp + "SofiaPro/SofiaPro.otf", 64));
		AddFont(Fonts, "Sofia_14", CreateFont(fp + "SofiaPro/SofiaPro.otf", 14));
		AddFont(Fonts, "Corbert_12", CreateFont(fp + "CorbertRegular/CorbertRegular.otf", 12));
		AddFont(Fonts, "Corbert_16", CreateFont(fp + "CorbertRegular/CorbertRegular.otf", 16));
		AddFont(Fonts, "Dense", CreateFont(fp + "DenseRegular/DenseRegular.otf", 64));
		AddFont(Fonts, "8Bit", CreateFont(fp + "8BitWonder/8BitWonder.ttf", 8));
		AddFont(Fonts, "8Bit_32", CreateFont(fp + "8BitWonder/8BitWonder.ttf", 32));
		AddFont(Fonts, "TallBold", CreateFont(fp + "TallBolder/TallBolder.ttf", 12));
		AddFont(Fonts, "BebasNeue", CreateFont(fp + "BebasNeue/BebasNeue.otf", 16));
		AddFont(Fonts, "BebasNeue_72", CreateFont(fp + "BebasNeue/BebasNeue.otf", 72));
		AddFont(Fonts, "BebasNeue_32", CreateFont(fp + "BebasNeue/BebasNeue.otf", 32));
		AddFont(Fonts, "WeblySleek_128", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 128));
		AddFont(Fonts, "WeblySleek_64", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 32));
		AddFont(Fonts, "WeblySleek_32", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 32));
		AddFont(Fonts, "WeblySleek_16", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 16));
		AddFont(Fonts, "WeblySleek_14", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 14));
		AddFont(Fonts, "WeblySleek_12", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 12));
		AddFont(Fonts, "WeblySleek_10", CreateFont(fp + "WeblySleek/weblysleekuisb.ttf", 10));
		AddFont(Fonts, "Reduction_24", CreateFont(fp + "Reduction/Reduction.ttf", 24));
		AddFont(Fonts, "Reduction_14", CreateFont(fp + "Reduction/Reduction.ttf", 14));
		AddFont(Fonts, "Reduction_10", CreateFont(fp + "Reduction/Reduction.ttf", 10));
		AddFont(Fonts, "VCR_OSD_MONO", CreateFont(fp + "VCR_OSD_MONO/VCR_OSD_MONO.ttf", 14));
		AddFont(Fonts, "Villeray_Semilight_14", CreateFont(fp + "Villeray/Villeray-Semilight.ttf", 14));
		AddFont(Fonts, "Villeray_Semilight_10", CreateFont(fp + "Villeray/Villeray-Semilight.ttf", 10));
		AddFont(Fonts, "Villeray_Bold_64", CreateFont(fp + "Villeray/Villeray-Bold.ttf", 64));
		AddFont(Fonts, "Villeray_Bold_10", CreateFont(fp + "Villeray/Villeray-Bold.ttf", 10));
		AddFont(Fonts, "Arrows7", CreateFont(fp + "Arrows7/Arrows7.ttf", 10));
		AddFont(Fonts, "Arrows7_12", CreateFont(fp + "Arrows7/Arrows7.ttf", 12));
		AddFont(Fonts, "Arrows7_24", CreateFont(fp + "Arrows7/Arrows7.ttf", 24));
		AddFont(Fonts, "Arrows7_32", CreateFont(fp + "Arrows7/Arrows7.ttf", 32));
		AddFont(Fonts, "CutOut", CreateFont(fp + "CutOut/CutOut.ttf", 12));
		AddFont(Fonts, "SilomBol_64", CreateFont(fp + "SilomBol/SilomBol.ttf", 64));
		AddFont(Fonts, "SilomBol_14", CreateFont(fp + "SilomBol/SilomBol.ttf", 14));
		AddFont(Fonts, "MisterPixelRegular_14", CreateFont(fp + "MisterPixel/MisterPixelRegular.ttf", 14));

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





