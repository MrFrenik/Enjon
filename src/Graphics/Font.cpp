#include <Math/Maths.h>

#include "Graphics/Font.h"
#include "Utils/Errors.h"

#include <stdexcept>

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace Enjon { namespace Fonts {

	// This Function Gets The First Power Of 2 >= The
	// Int That We Pass It.
	int next_p2 (int a )
	{
	    int rval=1;
	    // rval<<=1 Is A Prettier Way Of Writing rval*=2;
	    while(rval<a) rval<<=1;
	    return rval;
	}

	/* Inits a particular font with a particular size and stores in a returned map */
	void Init(char* filePath, GLuint size, Font* font)
	{
		// FreeType
	    FT_Library ft;
	    // All functions return a value different than 0 whenever an error occurred
	    if (FT_Init_FreeType(&ft))
	        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	    // Load font as face
	    FT_Face face;
	    if (FT_New_Face(ft, filePath, 0, &face))
	        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	    // Set size to load glyphs as
	    FT_Set_Pixel_Sizes(face, 0, size);

	    // Disable byte-alignment restriction
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	    // Load first 128 characters of ASCII set
	   
	    for (GLubyte c = 0; c < 128; c++)
	    {
	        // Load character glyph 
	        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
	        {
	            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
	            continue;
	        }
	        // Generate texture
	        GLuint texture;
	        glGenTextures(1, &texture);
	        glBindTexture(GL_TEXTURE_2D, texture);

	        glTexImage2D(
	        	GL_TEXTURE_2D,
	            0,
	            GL_RED,
	            face->glyph->bitmap.width,
	            face->glyph->bitmap.rows,
	            0,
	            GL_RED,
	            GL_UNSIGNED_BYTE,
	            face->glyph->bitmap.buffer
	        );

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glGenerateMipmap(GL_TEXTURE_2D);

	        // Now store character for later use
	        Character character = {
	            texture,
	            Enjon::Math::iVec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	            Enjon::Math::iVec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	            face->glyph->advance.x
	        };

	        // Insert into font character map
	        font->Characters.insert(std::pair<GLchar, Character>(c, character));

	        // Find max height
	        if (character.Size.y > font->MaxHeight) font->MaxHeight = character.Size.y;
	        if (character.Size.x > font->MaxWidth) font->MaxWidth = character.Size.x;

	        // Unbind texture
		    glBindTexture(GL_TEXTURE_2D, 0);
	    }

	    // Destroy FreeType once we're finished
	    FT_Done_Face(face);
	    FT_Done_FreeType(ft);
	}

	/* Creates and returns new font */
	Font* CreateFont(char* filePath, GLuint size)
	{
		// Create new font
		Font* F = new Font;

		// Set max width and height to 0
		F->MaxHeight = 0.0f;
		F->MaxWidth = 0.0f;

		// Init
		Init(filePath, size, F);

		// Check for null
		if (F == nullptr) Utils::FatalError("FONT::CREATE_FONT::Font is null.");

		return F;
	}

	/* Gets character stats from given font */
	CharacterStats GetCharacterAttributes(Enjon::Math::Vec2 Pos, float scale, Font* F, char c, float* advance)
	{
		Character ch = F->Characters[c];

		float x = Pos.x;
		float y = Pos.y;

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        Enjon::Math::Vec4 DestRect(xpos, ypos, w, h);
        Enjon::Math::Vec4 UV(0.00f, 1.0f, 1.0f, 0.98f);

        *advance = x + (ch.Advance >> 6) * scale;

        return CharacterStats{DestRect, UV, ch.TextureID};
	}

	float GetAdvance(char c, Font* F, float scale)
	{
		Character ch = F->Characters[c];

		return (ch.Advance >> 6) * scale;
	}

	float GetStringAdvance(const char* C, Font* F, float Scale)
	{
		float Advance = 0.0f;
	    std::string::const_iterator c;
	    std::string Text(C);
	    for (c = Text.begin(); c != Text.end(); c++)
	    {
	    	Advance += GetAdvance(*c, F, Scale);
	    } 
	    return Advance;
	}

	float GetHeight(char c, Font* F, float scale)
	{
		Character ch = F->Characters[c];

		return (ch.Size.y + ch.Bearing.y) * scale;
	}

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Font* F, SpriteBatch& Batch, ColorRGBA16 Color, TextStyle Style, float Angle, float Depth)
	{
	    if (Style == TextStyle::SHADOW) 
	    {
	    	PrintText(x + scale * 1.0f, y - 1.0f * scale, scale, text, F, Batch, RGBA16_Black(), TextStyle::DEFAULT, Angle, Depth);
	    }

		// Iterate through all characters
	    std::string::const_iterator c;
	    for (c = text.begin(); c != text.end(); c++) 
	    {
	        Character ch = F->Characters[*c];

	        GLfloat xpos = x + ch.Bearing.x * scale;
	        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

	        GLfloat w = ch.Size.x * scale;
	        GLfloat h = ch.Size.y * scale;

	        Enjon::Math::Vec4 DestRect(xpos, ypos, w, h);
	        Enjon::Math::Vec4 UV(0.00f, 0.05f, 1.0f, 0.90f);

	        // Add to batch
	        if (Angle) Batch.Add(DestRect, UV, ch.TextureID, Color, Depth, Angle);
	        else Batch.Add(DestRect, UV, ch.TextureID, Color, Depth);

	        // Advance to next character
	        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    }

	}

	void PrintText(EM::Transform& Transform, std::string Text, Font* F, QuadBatch& Batch, ColorRGBA16 Color, float Spacing, TextStyle Style)
	{
		EM::Vec3& Position = Transform.Position;
		EM::Quaternion& Rotation = Transform.Rotation;
		EM::Vec3& Scale = Transform.Scale;

		float x = Transform.Position.x;
		float y = Transform.Position.y;

	    // if (Style == TextStyle::SHADOW) 
	    // {
	    // 	PrintText(x + scale * 1.0f, y - 1.0f * scale, scale, text, F, Batch, RGBA16_Black(), TextStyle::DEFAULT, Angle, Depth);
	    // }

		// Iterate through all characters
	    std::string::const_iterator c;
	    for (c = Text.begin(); c != Text.end(); c++) 
	    {
	        Character ch = F->Characters[*c];

	        GLfloat xpos = x + ch.Bearing.x * Scale.x;
	        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * Scale.y;

	        GLfloat w = ch.Size.x;
	        GLfloat h = ch.Size.y;

	        Enjon::Math::Vec4 UV(0.00f, 0.05f, 1.0f, 0.90f);

	        // Add to batch
	        Batch.Add(
						EM::Vec2(w, h),
	        			EM::Transform(
	        							EM::Vec3(xpos, ypos, Position.z),
	        							Rotation,
	        							EM::Vec3(Scale.x, Scale.y, 1.0f)
	        						),
	        			UV,
	        			ch.TextureID, 
	        			Color
	        		);

	        // Advance to next character
	        x += (ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    }
	}

}}