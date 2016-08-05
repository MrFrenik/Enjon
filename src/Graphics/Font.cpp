#include <Math/Maths.h>

#include "Graphics/Font.h"
#include "Utils/Errors.h"

#include <stdexcept>

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace Enjon { namespace Graphics { namespace Fonts {

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

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
	    

	    /*
	    for (GLubyte c = 0; c < 128; c++)
	    {
		    // Load The Glyph For Our Character.
		    if(FT_Load_Glyph( face, FT_Get_Char_Index( face, c ), FT_LOAD_DEFAULT ))
		        throw std::runtime_error("FT_Load_Glyph failed");
		 
		    // Move The Face's Glyph Into A Glyph Object.
		    FT_Glyph glyph;
		    if(FT_Get_Glyph( face->glyph, &glyph ))
		        throw std::runtime_error("FT_Get_Glyph failed");
		 
		    // Convert The Glyph To A Bitmap.
		    FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
		    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

		    // This Reference Will Make Accessing The Bitmap Easier.
		    FT_Bitmap& bitmap=bitmap_glyph->bitmap;
		 
			// Use Our Helper Function To Get The Widths Of
			// The Bitmap Data That We Will Need In Order To Create
			// Our Texture.
			int width = next_p2( bitmap.width );
			int height = next_p2( bitmap.rows );
			 
			// Allocate Memory For The Texture Data.
			GLubyte* expanded_data = new GLubyte[ 2 * width * height];
			 
			// Here We Fill In The Data For The Expanded Bitmap.
			// Notice That We Are Using A Two Channel Bitmap (One For
			// Channel Luminosity And One For Alpha), But We Assign
			// Both Luminosity And Alpha To The Value That We
			// Find In The FreeType Bitmap.
			// We Use The ?: Operator To Say That Value Which We Use
			// Will Be 0 If We Are In The Padding Zone, And Whatever
			// Is The FreeType Bitmap Otherwise.
			for(int j=0; j <height;j++) 
			{
			    for(int i=0; i < width; i++)
			    {
			        expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] =
			            (i>=bitmap.width || j>=bitmap.rows) ?
			            0 : bitmap.buffer[i + bitmap.width*j];
			    }
			}
			// Now We Just Setup Some Texture Parameters.
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture( GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			 
			// Here We Actually Create The Texture Itself, Notice
			// That We Are Using GL_LUMINANCE_ALPHA To Indicate That
			// We Are Using 2 Channel Data.
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			    GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );
			
			 
			// With The Texture Created, We Don't Need The Expanded Data Anymore.
			delete [] expanded_data;
	    }
	    */

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
	CharacterStats GetCharacterAttributes(Enjon::Math::Vec2 Pos, float scale, Font* F, std::string::const_iterator c, float* advance)
	{
		Character ch = F->Characters[*c];

		float x = Pos.x;
		float y = Pos.y;

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        Enjon::Math::Vec4 DestRect(xpos, ypos, w, h);
        Enjon::Math::Vec4 UV(0, 0, 1, 1);

        *advance = x + (ch.Advance >> 6) * scale;

        return CharacterStats{DestRect, UV, ch.TextureID};
	}

	float GetAdvance(char c, Font* F, float scale)
	{
		Character ch = F->Characters[c];

		return (ch.Advance >> 6) * scale;
	}

	float GetHeight(char c, Font* F, float scale)
	{
		Character ch = F->Characters[c];

		return (ch.Size.y + ch.Bearing.y) * scale;
	}

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Font* F, Enjon::Graphics::SpriteBatch& Batch, Enjon::Graphics::ColorRGBA16 Color, TextStyle Style, float Angle, float Depth)
	{
	    if (Style == TextStyle::SHADOW) 
	    {
	    	PrintText(x + scale * 1.0f, y - 1.0f * scale, scale, text, F, Batch, EG::RGBA16_Black(), TextStyle::DEFAULT, Angle, Depth);
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
	        Enjon::Math::Vec4 UV(0, 0, 1, 1);

	        // Add to batch
	        if (Angle) Batch.Add(DestRect, UV, ch.TextureID, Color, Depth, Angle);
	        else Batch.Add(DestRect, UV, ch.TextureID, Color, Depth);

	        // Advance to next character
	        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    }

	}

}}}