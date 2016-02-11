#include <Math/Maths.h>

#include "Graphics/Font.h"

namespace Enjon { namespace Graphics { namespace Fonts {

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
	        // Set texture options
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	        // Now store character for later use
	        Character character = {
	            texture,
	            Enjon::Math::iVec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	            Enjon::Math::iVec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	            face->glyph->advance.x
	        };

	        // Insert into font character map`
	        font->Characters.insert(std::pair<GLchar, Character>(c, character));
	    }
	    glBindTexture(GL_TEXTURE_2D, 0);
	    // Destroy FreeType once we're finished
	    FT_Done_Face(face);
	    FT_Done_FreeType(ft);
	}

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Font* F, Enjon::Graphics::SpriteBatch& Batch, Enjon::Graphics::ColorRGBA16 Color)
	{
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
	        Enjon::Math::Vec4 UV(0, 0, 1, 1);

	        // Add to batch
	        Batch.Add(DestRect, UV, ch.TextureID, Color);

	        // Advance to next character
	        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    }
	}

}}}