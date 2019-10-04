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
		HashMap<GLchar, Character> Characters;
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
	void Init( const String& filePath, GLuint size, Font* font );

	/* Gets character stats from given font */
	// CharacterStats GetCharacterAttributes(Math::Vec2 Pos, float scale, Font* F, std::string::const_iterator c, float* advance);

	CharacterStats GetCharacterAttributes( const Vec2& Pos, const f32& scale, Font* F, char c, float* advance );

	float GetAdvance( char c, Font* F, const f32& scale = 1.0f );

	float GetStringAdvance( const char* C, Font* F, const f32& Scale = 1.0f );

	float GetHeight( char c, Font* F, const f32& scale = 1.0f );

	/* Creates and returns new font */
	Font* CreateFont( const String& filePath, u32 size );

	/* Adds a string of tex at (x,y) to given spritebatch */
	void PrintText( f32 x, f32 y, const f32& scale, const String& text, Font* F, SpriteBatch& Batch, 
						const ColorRGBA32& Color = RGBA32_White(), TextStyle Style = TextStyle::SHADOW, const f32& Angle = 0.0f, const f32& Depth = 0.0f );

	void PrintText( const Transform& Transform, const String& Text, Font* F, QuadBatch& Batch, const ColorRGBA32& Color = RGBA32_White(), const f32& Spacing = 1.0f, TextStyle Style = TextStyle::SHADOW ); 

	void PrintText( const Vec2& position, const Vec2& size, const String& Text, Font* F, SpriteBatch* Batch, const ColorRGBA32& Color = RGBA32_White(), const f32& Spacing = 1.0f, TextStyle Style = TextStyle::SHADOW ); 

	class UIFont;
	
	void PrintText( const Transform& Transform, const String& Text, const UIFont* F, const QuadBatch& Batch, const ColorRGBA32& Color, const u32& fontSize );

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
			FontAtlas( const String& path, const s32& fontSize, const UIFont* font );
			AssetHandle< Texture > GetAtlasTexture( ) const;
			u32 GetTextureID( ) const;
			FontGlyph GetGlyph( const u8& character ) const;
			~FontAtlas( ); 

		protected: 
			Enjon::AssetHandle< Texture > mAtlasTexture;
			u32 mAtlasTextureID;
			HashMap< u8, FontGlyph > mGlyphs;
	};

	class FontAssetLoader;

	//ENJON_CLASS( )
	//class UIFont : public Asset
	//{
	//	friend FontAssetLoader;
	//	friend FontAtlas;

	//	ENJON_CLASS_BODY( UIFont )

	//	public:
	//		/**
	//		* @brief Constructor
	//		*/
	//		UIFont( const String& fontPath ); 

	//		/**
	//		* @brief
	//		*/
	//		bool AtlasExists( const s32& fontSize ) const;

	//		/**
	//		* @brief
	//		*/
	//		const FontAtlas* GetAtlas( const s32& fontSize ) const;

	//	private:
	//		/**
	//		* @brief
	//		*/
	//		void AddAtlas( const s32& fontSize );

	//		FT_Face GetFace( ) const 
	//		{ 
	//			return mFontFace;  
	//		}

	//	private:
	//		ENJON_PROPERTY( )
	//		String mFontPath;

	//		HashMap< u32, FontAtlas > mAtlases; 

	//		FT_Face mFontFace;
	//};

	typedef struct FontData
	{
		u32 mSize;
		void* mData;
	} FontData;

	ENJON_CLASS( Construct )
	class UIFont : public Asset 
	{ 
		ENJON_CLASS_BODY( UIFont )
			
		/**
		* @brief Constructor
		*/
		UIFont( const String& fontPath ); 

		Result SerializeData( ByteBuffer* buffer ) const override;
		Result DeserializeData( ByteBuffer* buffer ) override;

		private: 
			FontData mFontData;
	};
}

#endif