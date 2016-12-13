#ifndef ENJON_FONT_H
#define ENJON_FONT_H

#include <unordered_map>

#include <GLEW/glew.h>

#include "Graphics/SpriteBatch.h"
#include "Graphics/Color.h"
#include "Math/Transform.h"
#include "Graphics/QuadBatch.h"

#include "ft2build.h"
#include FT_FREETYPE_H


namespace Enjon { namespace Graphics { namespace Fonts {

	enum TextStyle { DEFAULT, SHADOW };

	typedef struct 
	{
		GLuint TextureID;
		Enjon::Math::iVec2 Size;
		Enjon::Math::iVec2 Bearing;
		FT_Pos Advance;
	} Character;

	typedef struct 
	{
		std::unordered_map<GLchar, Character> Characters;
		float MaxHeight;
		float MaxWidth;	
	} Font;

	typedef struct 
	{
		Enjon::Math::Vec4 DestRect;
		Enjon::Math::Vec4 UV;
		GLuint TextureID;
	} CharacterStats;

	/* Inits a particular font with a particular size and stores in a returned map */
	void Init(char* filePath, GLuint size, Font* font);

	/* Gets character stats from given font */
	// CharacterStats GetCharacterAttributes(Math::Vec2 Pos, float scale, Font* F, std::string::const_iterator c, float* advance);

	CharacterStats GetCharacterAttributes(Enjon::Math::Vec2 Pos, float scale, Font* F, char c, float* advance);

	float GetAdvance(char c, Font* F, float scale = 1.0f);

	float GetStringAdvance(const char* C, Font* F, float Scale = 1.0f);

	float GetHeight(char c, Font* F, float scale = 1.0f);

	/* Creates and returns new font */
	Font* CreateFont(char* filePath, GLuint size);

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Font* F, Enjon::Graphics::SpriteBatch& Batch, 
						Enjon::Graphics::ColorRGBA16 Color = Enjon::Graphics::RGBA16_White(), TextStyle Style = TextStyle::SHADOW, float Angle = 0.0f, float Depth = 0.0f);

	void PrintText(EM::Transform& Transform, std::string Text, Font* F, EG::QuadBatch& Batch, EG::ColorRGBA16 Color = EG::RGBA16_White(), float Spacing = 1.0f, TextStyle Style = TextStyle::SHADOW);

}}}

#endif