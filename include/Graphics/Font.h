#ifndef ENJON_FONT_H
#define ENJON_FONT_H

#include <unordered_map>

#include <GLEW/glew.h>

#include "Graphics/SpriteBatch.h"
#include "Graphics/Color.h"
#include "Graphics/FontPack.h"
#include "Math/Transform.h"
#include "Graphics/QuadBatch.h"
#include "System/Types.h"
#include "Math/Vec2.h"
#include "Math/Vec4.h"
#include "Defines.h" 
#include "Graphics/Texture.h"
#include "Asset/Asset.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define MAX_NUMBER_GLYPHS 128 

namespace Enjon 
{ 
	enum class TextStyle 
	{ 
		DEFAULT, 
		SHADOW 
	};

	typedef struct 
	{
		GLuint TextureID;
		Enjon::iVec2 Size;
		Enjon::iVec2 Bearing;
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
		Vec4 DestRect;
		Vec4 UV;
		GLuint TextureID;
	} CharacterStats;

	typedef struct
	{
		Enjon::u8 character;
		rect_xywhf rect;
	} PackedCharacterRect;

	/* Inits a particular font with a particular size and stores in a returned map */
	void Init(const String& filePath, GLuint size, Font* font);

	/* Gets character stats from given font */
	// CharacterStats GetCharacterAttributes(Math::Vec2 Pos, float scale, Font* F, std::string::const_iterator c, float* advance);

	CharacterStats GetCharacterAttributes(Enjon::Vec2 Pos, float scale, Font* F, char c, float* advance);

	float GetAdvance(char c, Font* F, float scale = 1.0f);

	float GetStringAdvance(const char* C, Font* F, float Scale = 1.0f);

	float GetHeight(char c, Font* F, float scale = 1.0f);

	/* Creates and returns new font */
	Font* CreateFont(const String& filePath, GLuint size);

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText(GLfloat x, GLfloat y, GLfloat scale, std::string text, Font* F, Enjon::SpriteBatch& Batch, 
						ColorRGBA32 Color = Enjon::RGBA32_White(), TextStyle Style = TextStyle::SHADOW, float Angle = 0.0f, float Depth = 0.0f);

	void PrintText(Transform& Transform, std::string Text, Font* F, QuadBatch& Batch, ColorRGBA32 Color = RGBA32_White(), float Spacing = 1.0f, TextStyle Style = TextStyle::SHADOW); 

	void PrintText(const Vec2& position, const Vec2& size, std::string Text, Font* F, SpriteBatch* Batch, ColorRGBA32 Color = RGBA32_White(), float Spacing = 1.0f, TextStyle Style = TextStyle::SHADOW); 

	class UIFont;
	
	void PrintText( Transform& Transform, const Enjon::String& Text, const UIFont* F, QuadBatch& Batch, ColorRGBA32 Color, u32 fontSize );

	class UIFont;
	class FontAtlas;
	class FontGlyph
	{
		friend UIFont;
		friend FontAtlas;

		public:
			FontGlyph( );
			~FontGlyph( );
			Vec2 GetUVCoords( );
			Vec2 GetBearing( ) const;
			Vec2 GetSize( );
			Vec4 GetTextureCoords( ) const;
			s32 GetAdvance( ) const; 
			f32 GetWidth( ) const { return mWidth; }
			f32 GetHeight( ) const { return mHeight; }
			f32 GetLeft( ) const { return mLeft; }
			f32 GetTop( ) const { return mTop; }
			f32 GetUVOffsetX( ) const { return mUVOffsetX; }
			f32 GetUVOffsetY( ) const { return mUVOffsetY; }

		protected:
			FontAtlas*	mAtlas = nullptr;
			Vec4		mTextureCoordinates;
			Vec2		mBearing;
			f32			mXAdvance;
			f32			mYAdvance;
			f32			mWidth;
			f32			mHeight;
			f32			mLeft;
			f32			mTop;
			f32			mUVOffsetX;
			f32			mUVOffsetY;
	};

	class FontAtlas
	{
		friend UIFont;

		public:
			FontAtlas( );
			FontAtlas( const Enjon::String& path, s32 fontSize, const UIFont* font );
			Enjon::AssetHandle< Enjon::Texture > GetAtlasTexture( ) const;
			u32 GetTextureID( ) const;
			FontGlyph GetGlyph( u8 character ) const;
			~FontAtlas( ); 

		protected: 
			Enjon::AssetHandle< Enjon::Texture > mAtlasTexture;
			u32 mAtlasTextureID;
			std::unordered_map< u8, FontGlyph > mGlyphs;
	};

	class FontAssetLoader;

	ENJON_CLASS( )
	class UIFont : public Asset
	{
		friend FontAssetLoader;
		friend FontAtlas;

		ENJON_CLASS_BODY( UIFont )

		public:
			/**
			* @brief Constructor
			*/
			UIFont( const Enjon::String& fontPath ); 

			/**
			* @brief
			*/
			bool AtlasExists( s32 fontSize ) const;

			/**
			* @brief
			*/
			const FontAtlas* GetAtlas( s32 fontSize ) const;

		private:
			/**
			* @brief
			*/
			void AddAtlas( s32 fontSize );

			FT_Face GetFace( ) const 
			{ 
				return mFontFace;  
			}

		private:
			ENJON_PROPERTY( )
			Enjon::String mFontPath;

			std::unordered_map< u32, FontAtlas > mAtlases; 

			FT_Face mFontFace;
	};
}

#endif