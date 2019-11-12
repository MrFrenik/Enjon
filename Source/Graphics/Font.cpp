#include "Math/Maths.h"
#include "Graphics/Font.h"
#include "System/Types.h"
#include "Utils/Errors.h" 
#include "Asset/FontAssetLoader.h"
#include "ImGui/ImGuiManager.h" 
#include "ImGui/imgui_internal.h"

#include <stdexcept>
#include <stdlib.h>

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>


const Enjon::u32 GLYPH_SIZE = 128;

namespace Enjon 
{ 
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
	void Init( const String& filePath, GLuint size, Font* font )
	{
		// FreeType
	    FT_Library ft;
	    // All functions return a value different than 0 whenever an error occurred
	    if (FT_Init_FreeType(&ft))
	        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	    // Load font as face
	    FT_Face face;
	    if (FT_New_Face(ft, filePath.c_str(), 0, &face))
	        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		
		// Used for atlas packing
		rect_xywhf* rects = new rect_xywhf[GLYPH_SIZE];
		rect_xywhf* ptr_rects[GLYPH_SIZE];

	    // Set size to load glyphs as
	    FT_Set_Pixel_Sizes(face, 0, size);

	    // Disable byte-alignment restriction
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	    // Load first 128 characters of ASCII set
	   
		u32 i = 0;
		s32 maxSide = 500;
		s32 maxWidth = 0;
		s32 maxHeight = 0;
	    for (GLubyte c = 0; c < 128; c++, ++i)
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

			glGenerateMipmap(GL_TEXTURE_2D);

	        // Now store character for later use
	        Character character = {
	            texture,
	            iVec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	            iVec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	            face->glyph->advance.x
	        };

			if ( character.Size.x > maxWidth )
			{
				maxWidth = character.Size.x;
			}

			if ( character.Size.y > maxHeight )
			{
				maxHeight = character.Size.y;
			}

	        // Insert into font character map
	        font->Characters.insert(std::pair<GLchar, Character>(c, character));

	        // Find max height
	        if (character.Size.y > font->MaxHeight) font->MaxHeight = character.Size.y;
	        if (character.Size.x > font->MaxWidth) font->MaxWidth = character.Size.x;

			rects[i] = rect_xywhf( 0, 0, character.Size.x, character.Size.y, c );
			ptr_rects[i] = rects + i;

	        // Unbind texture
		    glBindTexture(GL_TEXTURE_2D, 0);
	    } 

		// Need to pack this into atlas now
		//GLubyte* texData = new GLubyte[ maxWidth * 128 * maxHeight];
		//for ( GLubyte c = 0; i < 128; ++c ) 
		//{
		//	// Grab character
		//	Character ch = font->Characters[c]; 


		//	
		//}

		// Pack the bins
		/*
		if ( pack( ptr_rects, GLYPH_SIZE, maxSide, font->Atlas.bins ) )
		{
			font->Atlas.size.x = font->Atlas.bins.at( 0 ).size.w;
			font->Atlas.size.y = font->Atlas.bins.at( 0 ).size.h;

			u32 bufferSize = font->Atlas.bins.at( 0 ).size.w * font->Atlas.bins.at( 0 ).size.h;
			
			// Create texData 
			GLubyte* texData = new GLubyte[bufferSize];

			// Fill tex data with rect data
			std::vector< rect_xywhf* >* rects = &font->Atlas.bins.at( 0 ).rects;
			for ( u32 i = 0; i < rects->size( ); ++i )
			{
				rect_xywhf* rect = rects->at( i );

				// Get character
				char c = rect->c; 

				// Load character glyph from face again
				if ( FT_Load_Char( face, c, FT_LOAD_RENDER ) )
				{
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					continue;
				}

				//bool flipped_rect = false;
				//if ( rect->flipped )
				//{
				//	rect->flip( );
				//	flipped_rect = true;
				//}

				for ( u32 row = rect->y; row < rect->y + rect->h; ++row )
				{
					for ( u32 col = rect->x; col < rect->x + rect->w; ++col )
					{
						u32 idx = row * font->Atlas.bins.at( 0 ).size.w + col; 
						u32 idx2 = row * face->glyph->bitmap.width + col;
						if ( idx == idx2 )
						{
							texData[idx] = face->glyph->bitmap.buffer[idx]; 
						}
					}
				}
			}

			glGenTextures( 1, &font->Atlas.textureID );
			glBindTexture( GL_TEXTURE_2D, font->Atlas.textureID );

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				font->Atlas.size.x,
				font->Atlas.size.y,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				texData
			);

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

			glGenerateMipmap( GL_TEXTURE_2D ); 
		}
		*/

	    // Destroy FreeType once we're finished
	    FT_Done_Face(face);
	    FT_Done_FreeType(ft);

		delete[] rects;
		rects = nullptr;
	}

	/* Creates and returns new font */
	Font* CreateFont( const String& filePath, u32 size )
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
	CharacterStats GetCharacterAttributes( const Vec2& Pos, const f32& scale, Font* F, char c, f32* advance )
	{
		Character ch = F->Characters[c];

		f32 x = Pos.x;
		f32 y = Pos.y;

        f32 xpos = x + ch.Bearing.x * scale;
        f32 ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        f32 w = ch.Size.x * scale;
        f32 h = ch.Size.y * scale;

        Vec4 DestRect(xpos, ypos, w, h);
        Vec4 UV(0.00f, 1.0f, 1.0f, 1.0f);

        *advance = x + (ch.Advance >> 6) * scale;

        return CharacterStats{DestRect, UV, ch.TextureID};
	}

	f32 GetAdvance( char c, Font* F, const f32& scale )
	{
		Character ch = F->Characters[c];

		return (ch.Advance >> 6) * scale;
	}

	f32 GetStringAdvance( const char* C, Font* F, const f32& Scale )
	{
		f32 Advance = 0.0f;
	    String::const_iterator c;
	    String Text(C);
	    for (c = Text.begin(); c != Text.end(); c++)
	    {
	    	Advance += GetAdvance(*c, F, Scale);
	    } 
	    return Advance;
	}

	f32 GetHeight( char c, Font* F, const f32& scale )
	{
		Character ch = F->Characters[c];

		return (ch.Size.y + ch.Bearing.y) * scale;
	}

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText( f32 x, f32 y, const f32& scale, const String& text, Font* F, SpriteBatch& Batch, const ColorRGBA32& Color, TextStyle Style, const f32& Angle, const f32& Depth )
	{
	    if (Style == TextStyle::SHADOW) 
	    {
	    	PrintText(x + scale * 1.0f, y - 1.0f * scale, scale, text, F, Batch, RGBA32_Black(), TextStyle::DEFAULT, Angle, Depth + 0.1f);
	    }

		// Iterate through all characters
	    String::const_iterator c;
	    for (c = text.begin(); c != text.end(); c++) 
	    {
	        Character ch = F->Characters[*c];

	        f32 xpos = x + ch.Bearing.x * scale;
	        f32 ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

	        f32 w = ch.Size.x * scale;
	        f32 h = ch.Size.y * scale;

	        Vec4 DestRect(xpos, ypos, w, h);
	        Vec4 UV(0.00f, 0.0f, 1.0f, 1.0f);

	        // Add to batch
	        if (Angle) Batch.Add(DestRect, UV, ch.TextureID, Color, Depth, Angle);
	        else Batch.Add(DestRect, UV, ch.TextureID, Color, Depth);

	        // Advance to next character
	        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    }

	}

	void PrintText( const Vec2& position, const Vec2& size, const String& Text, Font* F, SpriteBatch* Batch, const ColorRGBA32& Color, const f32& Spacing, TextStyle Style )
	{ 
		f32 x = position.x;
		f32 y = position.y; 

		// Iterate through all characters
	    String::const_iterator c;
	    for (c = Text.begin(); c != Text.end(); c++) 
	    {
	        Character ch = F->Characters[*c];

	        f32 xpos = x + ch.Bearing.x * size.x;
	        f32 ypos = y - (ch.Size.y - ch.Bearing.y) * size.y;

	        f32 w = ch.Size.x;
	        f32 h = ch.Size.y;

	        Vec4 UV(0.00f, 0.05f, 1.0f, 0.90f); 

	        // Add to batch
			Batch->Add( Vec4( xpos, ypos, size.x, size.y ), UV, ch.TextureID, Color ); 

	        // Advance to next character
	        x += (ch.Advance >> 6) * size.x; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    } 
	}

	void PrintText( const Transform& transform, const String& text, Font* font, QuadBatch& batch, const ColorRGBA32& color, const f32& spacing, TextStyle style )
	{
		Vec3 position = transform.GetPosition();
		Quaternion rotation = transform.GetRotation();
		Vec3 scale = transform.GetScale();

		f32 x = transform.GetPosition().x;
		f32 y = transform.GetPosition().y;

	    // if (Style == TextStyle::SHADOW) 
	    // {
	    // 	PrintText(x + scale * 1.0f, y - 1.0f * scale, scale, text, F, Batch, RGBA32_Black(), TextStyle::DEFAULT, Angle, Depth);
	    // }

		// Iterate through all characters
	    String::const_iterator c;
	    for (c = text.begin(); c != text.end(); c++) 
	    {
	        Character ch = font->Characters[*c];

	        f32 xpos = x + ch.Bearing.x * scale.x;
	        f32 ypos = y - (ch.Size.y - ch.Bearing.y) * scale.y;

	        f32 w = ch.Size.x;
	        f32 h = ch.Size.y;

	        Vec4 uv(0.00f, 0.05f, 1.0f, 0.90f); 

	        // Add to batch
	        batch.Add(
						Vec2(w, h),
	        			Enjon::Transform(
	        							Vec3(xpos, ypos, position.z),
	        							rotation,
	        							Vec3(scale.x, scale.y, 1.0f)
	        						),
	        			uv,
	        			ch.TextureID, 
	        			color
	        		);

	        // Advance to next character
	        x += (ch.Advance >> 6) * scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    }
	}

	void PrintText( const Transform& transform, const String& text, const UIFont* font, QuadBatch& batch, const ColorRGBA32& color, const u32& fontSize )
	{ 
		//const FontAtlas* atlas = font->GetAtlas( (s32)fontSize );
		//f32 textureWidth = atlas->GetAtlasTexture( ).Get( )->GetWidth( );
		//f32 textureHeight = atlas->GetAtlasTexture( ).Get( )->GetHeight( ); 

		//Vec3 scale = transform.GetScale( );
		//Vec3 position = transform.GetPosition( );
		//f32 x = position.x;
		//f32 y = position.y;

		//String::const_iterator c;
		//for ( c = text.begin( ); c != text.end( ); ++c )
		//{
		//	FontGlyph glyph = atlas->GetGlyph( *c );
		//	f32 width = glyph.GetWidth( );
		//	f32 height = glyph.GetHeight( );

		//	f32 u1 = glyph.GetUVOffsetX( );
		//	f32 v1 = 1.0f - glyph.GetUVOffsetY( );
		//	f32 u2 = u1 + ( width / textureWidth );
		//	f32 v2 = v1 - ( height / textureHeight );

		//	f32 w = width;
		//	f32 h = height;

		//	Vec2 bearing = glyph.GetBearing( );
		//	f32 xpos = x + ( bearing.x * scale.x );
		//	f32 ypos = y - ( height - bearing.y ) * scale.y;

		//	Vec4 uv( u1, v2, u2, v1 );
		//	//Vec4 uv( 0, 0, 1, 1 );

		//	Transform tform;
		//	tform.SetPosition( Vec3( x, y, position.z ) );
		//	tform.SetRotation( transform.GetRotation( ) );
		//	tform.SetScale( Vec3( scale.x, scale.y, 1.0f ) );

		//	AssetHandle< Texture > atlasTexture = atlas->GetAtlasTexture( );
		//	batch.Add( Vec2( w, h ), tform, uv, atlasTexture.Get( )->GetTextureId( ) );

		//	s32 advance = glyph.GetAdvance( );
		//	x += (f32) advance * scale.x;
		//}
	}

	//========================================================================================================================

	FontGlyph::FontGlyph( )
	{

	}

	//========================================================================================================================

	FontGlyph::~FontGlyph( )
	{

	}

	//========================================================================================================================
			
	Vec2 FontGlyph::GetUVCoords( )
	{
		u32 width = mAtlas->GetAtlasTexture( ).Get( )->GetWidth( );
		u32 height = mAtlas->GetAtlasTexture( ).Get( )->GetHeight( );
		return Vec2( (f32)mTextureCoordinates.x / (f32)width , (f32)mTextureCoordinates.y / (f32)height );
	}

	//========================================================================================================================

	Vec4 FontGlyph::GetTextureCoords( ) const
	{
		return mTextureCoordinates;
	}

	//========================================================================================================================

	Vec2 FontGlyph::GetBearing( ) const
	{
		return mBearing;
	}

	//========================================================================================================================

	Vec2 FontGlyph::GetSize( )
	{
		u32 width = mAtlas->GetAtlasTexture( ).Get( )->GetWidth( );
		u32 height = mAtlas->GetAtlasTexture( ).Get( )->GetHeight( );
		return Vec2( f32( mTextureCoordinates.z - mTextureCoordinates.x ) / (f32)width, f32( mTextureCoordinates.w - mTextureCoordinates.y ) / (f32)height );
	}

	//========================================================================================================================

	s32 FontGlyph::GetAdvance( ) const
	{
		return mXAdvance;
	}

	//========================================================================================================================

	FontAtlas::FontAtlas( )
	{ 
	}

	//========================================================================================================================

	FontAtlas::FontAtlas( const String& filePath, const s32& fontSize, const UIFont* font )
	{ 
		//if ( font == nullptr )
		//{
		//	return;
		//}

		//// Get font face
		//FT_Face face = font->GetFace( );

		//// Set size to load glyphs as
		//FT_Set_Pixel_Sizes( font->GetFace( ), 0, fontSize ); 

		//// Get dimentions
		//s32 maxDim = ( 1 + ( face->size->metrics.height >> 6 ) ) * std::ceilf( std::sqrtf( MAX_NUMBER_GLYPHS ) );
		//s32 texWidth = 1;
		//while ( texWidth < maxDim )
		//{
		//	texWidth <<= 1;
		//} 
		//s32 texHeight = texWidth;

		//// Render glyphs to atlas	
		//char* pixels = (char*) calloc( texWidth * texHeight, 1 );
		//s32 penX = 0; 
		//s32 penY = 0;

		//// Iterate through glyphs and build data
		//for ( u32 i = 0; i < MAX_NUMBER_GLYPHS; ++i )
		//{
		//	FT_Load_Char( face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT );
		//	FT_Bitmap* bmp = &face->glyph->bitmap; 

		//	if ( penX + bmp->width >= texWidth )
		//	{
		//		penX = 0;
		//		penY += ( ( face->size->metrics.height >> 6 ) + 1 );
		//	}

		//	// Build glyph
		//	for ( u32 row = 0; row < bmp->rows; ++row )
		//	{
		//		for ( u32 col = 0; col < bmp->width; ++col )
		//		{
		//			s32 x = penX + col;
		//			s32 y = penY + row;
		//			pixels[ y * texWidth + x ] = bmp->buffer[ row * bmp->pitch + col ];
		//		}
		//	}

		//	// Add glyph data
		//	FontGlyph gl;
		//	mGlyphs[ i ] = gl;
		//	FontGlyph* glyph = &mGlyphs[ i ];
		//	glyph->mTextureCoordinates.x	= penX / (f32)texWidth;
		//	glyph->mTextureCoordinates.y	= penY /(f32)texHeight;
		//	glyph->mTextureCoordinates.z	= penX + bmp->width;
		//	glyph->mTextureCoordinates.w	= penY + bmp->rows;
		//	glyph->mBearing.x				= face->glyph->bitmap_left;
		//	glyph->mBearing.y				= face->glyph->bitmap_top; 
		//	glyph->mXAdvance				= (f32)(face->glyph->advance.x >> 6); 
		//	glyph->mAtlas					= this;

		//	glyph->mWidth = face->glyph->bitmap.width;
		//	glyph->mHeight = face->glyph->bitmap.rows;

		//	glyph->mLeft = face->glyph->bitmap_left;
		//	glyph->mTop = face->glyph->bitmap_top;

		//	glyph->mUVOffsetX = penX / (f32) texWidth;
		//	glyph->mUVOffsetY = penY / (f32) texHeight;

		//	penX += bmp->width + 1;
		//} 

		//char* textureData = ( char* ) calloc( texWidth * texHeight * 4, 1 );
		//for ( s32 i = 0; i < ( texWidth * texHeight ); ++i )
		//{
		//	textureData[ i * 4 + 0 ] |= pixels[ i ];
		//	textureData[ i * 4 + 1 ] |= pixels[ i ];
		//	textureData[ i * 4 + 2 ] |= pixels[ i ];
		//	textureData[ i * 4 + 3 ] |= pixels[ i ];
		//}
		//
		//// Disable byte-alignment restriction
		////glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); 

		//// Construct texture
		//glGenTextures( 1, &mAtlasTextureID );
		//glBindTexture( GL_TEXTURE_2D, mAtlasTextureID );

		//glTexImage2D(
		//	GL_TEXTURE_2D,
		//	0,
		//	GL_RGBA8,
		//	texWidth,
		//	texHeight,
		//	0,
		//	GL_RGBA,
		//	GL_UNSIGNED_BYTE,
		//	textureData
		//);

		//// Set up texture params
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

		//// Generate mipmaps
		//glGenerateMipmap( GL_TEXTURE_2D );

		//// Unbind texture 
		//glBindTexture( GL_TEXTURE_2D, 0 );
		//
		//Enjon::AssetHandle< Enjon::Texture > texture; 
		//texture.Set( new Texture( texWidth, texHeight, mAtlasTextureID ) ); 
		//mAtlasTexture = texture;

		//// Free all texture data
		//free( textureData );
		//free( pixels );
	}
			
	/*
	FontAtlas::FontAtlas( const Enjon::String& filePath, s32 fontSize )
	{
		// FreeType library
		FT_Library ft;

		// All functions return a value different than 0 whenever an error occurred
		if ( FT_Init_FreeType( &ft ) )
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		}

		// Load font as face
		FT_Face face;
		if ( FT_New_Face( ft, filePath.c_str( ), 0, &face ) )
		{
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		}

		const u32 MAX_WIDTH = 512;

		// Set face for later use
		mFontFace = face;

		FT_Set_Pixel_Sizes( mFontFace, 0, fontSize );
		FT_GlyphSlot glyphSlot = face->glyph;

		int roww = 0;
		int rowh = 0;
		int width = 0;
		int height = 0;

		for ( int i = 0; i < MAX_NUMBER_GLYPHS; i++ )
		{
			if ( FT_Load_Char( face, i, FT_LOAD_RENDER ) )
			{
				std::cout << "Loading character %c failed\n", i;
				continue;
			}

			if ( roww + glyphSlot->bitmap.width + 1 >= MAX_WIDTH )
			{
				width = std::fmax( width, roww );
				height += rowh;
				roww = 0;
				rowh = 0;
			}

			roww += glyphSlot->bitmap.width + 1;
			rowh = std::fmax( rowh, glyphSlot->bitmap.rows );
		}

		width = std::fmax( width, roww );
		height += rowh;

		glGenTextures( 1, &mAtlasTextureID );

		if ( glGetError( ) != GL_NO_ERROR )
		{
			std::cout << "glGenTextures failed\n";
		}

		glActiveTexture( GL_TEXTURE0 );

		//if ( glGetError( ) != GL_NO_ERROR )
		//{
		//	std::cout << "glActiveTexture failed\n";
		//}

		glBindTexture( GL_TEXTURE_2D, mAtlasTextureID );


		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0 );

		

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		//if ( glGetError( ) != GL_NO_ERROR )
		//{
		//	std::cout << "glPixelStorei failed\n";
		//}

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		

		int ox = 0;
		int oy = 0;

		rowh = 0;

		for ( int i = 0; i < MAX_NUMBER_GLYPHS; i++ )
		{
			if ( FT_Load_Char( face, i, FT_LOAD_RENDER ) )
			{
				std::cout << "Loading character %c failed\n", i;
				continue;
			}

			if ( ox + glyphSlot->bitmap.width + 1 >= MAX_WIDTH )
			{
				oy += rowh;
				rowh = 0;
				ox = 0;
			}

			glTexSubImage2D( GL_TEXTURE_2D, 0, ox, oy, glyphSlot->bitmap.width, glyphSlot->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, glyphSlot->bitmap.buffer );

			if ( glGetError( ) != GL_NO_ERROR )
			{
				std::cout << "BORKED AGAIN\n";
			}

			mGlyphs[ i ].mXAdvance = glyphSlot->advance.x >> 6;
			mGlyphs[ i ].mYAdvance = glyphSlot->advance.y >> 6;

			mGlyphs[ i ].mWidth = glyphSlot->bitmap.width;
			mGlyphs[ i ].mHeight = glyphSlot->bitmap.rows;

			mGlyphs[ i ].mLeft = glyphSlot->bitmap_left;
			mGlyphs[ i ].mTop = glyphSlot->bitmap_top;

			mGlyphs[ i ].mUVOffsetX = ox / (f32) width;
			mGlyphs[ i ].mUVOffsetY = oy / (f32) height;

			mGlyphs[ i ].mAtlas = this;

			rowh = std::fmax( rowh, glyphSlot->bitmap.rows );
			ox += glyphSlot->bitmap.width + 1;
		}

		// Generate mipmaps
		glGenerateMipmap( GL_TEXTURE_2D );

		// Unbind texture 
		glBindTexture( GL_TEXTURE_2D, 0 );

		// Create texture and assign it
		Enjon::AssetHandle< Enjon::Texture > texture; 
		texture.Set( new Texture( width, height, mAtlasTextureID ) );
		mAtlasTexture = texture;
	}
	*/

	//========================================================================================================================

	FontAtlas::~FontAtlas( )
	{

	}
			
	//========================================================================================================================

	FontGlyph FontAtlas::GetGlyph( const u8& character ) const
	{
		//// Grab character index
		//u32 index = FT_Get_Char_Index( mFontFace, character );
		//
		//// Make sure is in range
		assert( character < MAX_NUMBER_GLYPHS ); 

		auto query = mGlyphs.find( character );
		if ( query != mGlyphs.end( ) )
		{
			return query->second;
		}

		// Shouldn't ever hit here
		assert( false );

		FontGlyph gl;
		return gl;

		//// Return glyph
		//return mGlyphs[ index ]; 
	}
			
	//========================================================================================================================
			
	AssetHandle< Texture > FontAtlas::GetAtlasTexture( ) const
	{
		return mAtlasTexture;
	}
			
	//========================================================================================================================
	
	u32 FontAtlas::GetTextureID( ) const
	{
		return mAtlasTextureID;
	}

	//========================================================================================================================

	//UIFont::UIFont( const String& fontPath )
	//	: mFontPath( fontPath )
	//{ 
	//	// FreeType library
	//	FT_Library ft;

	//	// All functions return a value different than 0 whenever an error occurred
	//	if ( FT_Init_FreeType( &ft ) )
	//	{
	//		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl; 
	//	}

	//	// Load font as face
	//	if ( FT_New_Face( ft, fontPath.c_str( ), 0, &mFontFace ) )
	//	{
	//		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl; 
	//	} 
	//} 

	////========================================================================================================================

	//bool UIFont::AtlasExists( const s32& fontSize ) const
	//{
	//	// If not found, then will reach end of atlas map and return false
	//	return ( ( mAtlases.find( fontSize ) ) != mAtlases.end( ) ); 
	//}

	////========================================================================================================================
	//		
	//const FontAtlas* UIFont::GetAtlas( const s32& fontSize ) const
	//{
	//	UIFont* f = const_cast< UIFont* > ( this );

	//	// Build atlas if doesn't exist
	//	if ( !AtlasExists( fontSize ) )
	//	{
	//		f->AddAtlas( fontSize );
	//	}

	//	return &f->mAtlases[ fontSize ];
	//}

	////========================================================================================================================
	//		
	//void UIFont::AddAtlas( const s32& fontSize )
	//{
	//	// If doesn't exist, then place in map
	//	if ( !AtlasExists( fontSize ) )
	//	{
	//		FontAtlas atlas( mFontPath, fontSize, this );
	//		mAtlases[ fontSize ] = atlas;
	//	}
	//}

	//========================================================================================================================

	UIFont::UIFont( const String& fontPath )
	{
		// Do something with imgui, dur
		mFontData.mData = (u8*)ImFileLoadToMemory( fontPath.c_str(), "rb", (int*)&mFontData.mSize, 0); 
	}

	//======================================================================================================================== 

	Result UIFont::SerializeData( ByteBuffer* buffer ) const
	{ 
		// Write out font data size
		buffer->Write< u32 >( mFontData.mSize ); 
		// Write out font data
		for ( u32 i = 0; i < mFontData.mSize; ++i )
		{
			u8 b = (mFontData.mData)[ i ];
			buffer->Write< u8 >( b );
		}

		return Result::SUCCESS;
	}

	//======================================================================================================================== 

	Result UIFont::DeserializeData( ByteBuffer* buffer )
	{
		// Read in font data size
		mFontData.mSize = buffer->Read< u32 >( ); 
		// Read in font data
		mFontData.mData = (u8* )malloc( mFontData.mSize );
		for ( u32 i = 0; i < mFontData.mSize; ++i )
		{
			u8 b = buffer->Read< u8 >();
			mFontData.mData[i] = b;
		}

		return Result::SUCCESS;
	}

	//======================================================================================================================== 

	const FontData& UIFont::GetFontData() const
	{
		return mFontData;
	}

	//======================================================================================================================== 

}