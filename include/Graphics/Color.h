#ifndef ENJON_COLOR_H
#define ENJON_COLOR_H

#include <GLEW/glew.h> 
#include <Math/Vec4.h>
#include <Defines.h>

namespace Enjon 
{ 
	// Strict POD ColorRGBA8 struct
	struct ColorRGBA8
	{
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte a; 
	};

	// Initializer
	static inline ColorRGBA8 RGBA8(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
	{
		ColorRGBA8 color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;

		return color;
	}
	
	// Initializer
	static inline ColorRGBA8 RGBA8(GLubyte val)
	{
		ColorRGBA8 color;
		color.r = val;
		color.g = val;
		color.b = val;
		color.a = 255;

		return color;
	}

	// Initializer
	static inline ColorRGBA8 RGBA8(GLubyte val, float a)
	{
		ColorRGBA8 color;
		color.r = val;
		color.g = val;
		color.b = val;
		color.a = (int)(a * 255);

		return color;
	}

	// Predefined Color Types
	static ColorRGBA8 inline RGBA8_White()          { return RGBA8(255); }
	static ColorRGBA8 inline RGBA8_Black()          { return RGBA8(0, 0, 0, 255); }
	static ColorRGBA8 inline RGBA8_DarkGrey()       { return RGBA8(51, 51, 51, 255); }
	static ColorRGBA8 inline RGBA8_MidGrey()        { return RGBA8(128, 128, 128, 255); }
	static ColorRGBA8 inline RGBA8_LightGrey()      { return RGBA8(204, 204, 204, 255); }
	static ColorRGBA8 inline RGBA8_Red()            { return RGBA8(255, 0, 0, 255); }
	static ColorRGBA8 inline RGBA8_Green()          { return RGBA8(0, 255, 0, 255); }
	static ColorRGBA8 inline RGBA8_Blue()           { return RGBA8(0, 0, 255, 255); }
	static ColorRGBA8 inline RGBA8_Yellow()         { return RGBA8(0, 255, 255, 255); }
	static ColorRGBA8 inline RGBA8_Orange()         { return RGBA8(204, 77, 51, 255); }
	static ColorRGBA8 inline RGBA8_Magenta()        { return RGBA8(255, 0, 255, 255); }
	static ColorRGBA8 inline RGBA8_LightBlue()      { return RGBA8(51, 77, 179, 255); }
	static ColorRGBA8 inline RGBA8_Purple()			{ return RGBA8(255, 0, 255, 255); } 

	// Helper functions 
	inline bool operator==(const ColorRGBA8& left, const ColorRGBA8& other)
	{
		return (left.r == other.r && left.g == other.g && left.b == other.b && left.a == other.a);
	} 

	inline ColorRGBA8 SetOpacity(ColorRGBA8& left, float a)
	{
		return RGBA8(left.r, left.g, left.b, (int)(a * 255));
	}

	// Strict POD struct ColoRGBA32
	class ColorRGBA32
	{ 
		public:
			ColorRGBA32() 
				: r(1), g(1), b(1), a(1) 
			{}

			ColorRGBA32(float _r, float _g, float _b, float _a)
				: r(_r), g(_g), b(_b), a(_a)
			{}

			ColorRGBA32(const Vec4& V) : r(V.x), g(V.y), b(V.z), a(V.w) {}

			inline friend b8 operator==( const ColorRGBA32& left, const ColorRGBA32& right )
			{
				return ( left.r == right.r &&
						 left.g == right.g &&
						 left.b == right.b &&
						 left.a == right.a );
			}

			inline friend b8 operator!=( const ColorRGBA32& left, const ColorRGBA32& right )
			{
				return !( left == right );
			}

			float r;
			float g;
			float b;
			float a; 
	};

	// Initializer
	inline ColorRGBA32 RGBA32(float r, float g, float b, float a)
	{
		ColorRGBA32 color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;

		return color;
	}

	// Initializer
	inline ColorRGBA32 RGBA32(float val)
	{
		ColorRGBA32 color;
		color.r = val;
		color.b = val;
		color.g = val;
		color.a = 1.0f;

		return color;
	}
	
	// Initializer
	inline ColorRGBA32 RGBA32(float val, float a)
	{
		ColorRGBA32 color;
		color.r = val;
		color.b = val;
		color.g = val;
		color.a = a;

		return color;
	}
	
	// Predefined Color Types
	static ColorRGBA32 inline RGBA32_White()                { return RGBA32(1.0f); }
	static ColorRGBA32 inline RGBA32_Black()                { return RGBA32(0.0f, 0.0f, 0.0f, 1.0f); }
	static ColorRGBA32 inline RGBA32_LightGrey()		    { return RGBA32(0.8f, 0.8f, 0.8f, 1.0f); }
	static ColorRGBA32 inline RGBA32_MidGrey()              { return RGBA32(0.5f, 0.5f, 0.5f, 1.0f); } 
	static ColorRGBA32 inline RGBA32_DarkGrey()             { return RGBA32(0.1f, 0.1f, 0.1f, 1.0f); }
	static ColorRGBA32 inline RGBA32_Red()			        { return RGBA32(1.0f, 0.0f, 0.0f, 1.0f); }
	static ColorRGBA32 inline RGBA32_Green()				{ return RGBA32(0.0f, 1.0f, 0.0f, 1.0f); }
	static ColorRGBA32 inline RGBA32_Blue()					{ return RGBA32(0.0f, 0.0f, 1.0f, 1.0f); }
	static ColorRGBA32 inline RGBA32_Yellow()               { return RGBA32(1.0f, 0.8f, 0.0f, 1.0f); }
	static ColorRGBA32 inline RGBA32_Magenta()              { return RGBA32(1.0f, 0.0f, 1.0f, 1.0f); } 
	static ColorRGBA32 inline RGBA32_Orange()	     	    { return RGBA32(0.8f, 0.3f, 0.2f, 1.0f); }

	static ColorRGBA32 inline RGBA32_LightPurple() 			{ return RGBA32(0.24f, 0.22f, 0.35f, 1.0f); }
	static ColorRGBA32 inline RGBA32_LimeGreen() 			{ return RGBA32(0.49f, 1.0f, 0.51f, 1.0f); }
	static ColorRGBA32 inline RGBA32_BurntOrange()	     	{ return RGBA32(0.9f, 0.5f, 0.2f, 1.0f); }
	static ColorRGBA32 inline RGBA32_SkyBlue()              { return RGBA32(0.2f, 0.3f, 0.8f, 1.0f); } 
	static ColorRGBA32 inline RGBA32_ZombieGreen()          { return RGBA32(0.33f, 0.77f, 0.44f, 1.0f); }
	static ColorRGBA32 inline RGBA32_DullMagenta()          { return RGBA32(0.77f, 0.33f, 0.66f, 1.0f); }

	// Helper functions
	inline ColorRGBA32 SetOpacity(const ColorRGBA32& color, float a)   
	{ 
		return RGBA32(color.r, color.g, color.b, a); 
	}
}


#endif