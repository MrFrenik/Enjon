#ifndef ENJON_COLOR_H
#define ENJON_COLOR_H

#include <GLEW/glew.h> 

namespace Enjon { namespace Graphics { 

	// Strict POD ColorRGBA8 struct
	struct ColorRGBA8
	{
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte a; 
	};

	// Initializer
	inline ColorRGBA8 RGBA8(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
	{
		ColorRGBA8 color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;

		return color;
	}
	
	// Initializer
	inline ColorRGBA8 RGBA8(GLubyte val)
	{
		ColorRGBA8 color;
		color.r = val;
		color.g = val;
		color.b = val;
		color.a = 255;

		return color;
	}

	// Initializer
	inline ColorRGBA8 RGBA8(GLubyte val, float a)
	{
		ColorRGBA8 color;
		color.r = val;
		color.g = val;
		color.b = val;
		color.a = (int)(a * 255);

		return color;
	}

	// Predefined Color Types
	static ColorRGBA8 inline RGBA8_White()            { return RGBA8(255); }
	static ColorRGBA8 inline RGBA8_Black()            { return RGBA8(0, 0, 0, 255); }
	static ColorRGBA8 inline RGBA8_DarkGrey()         { return RGBA8(51, 51, 51, 255); }
	static ColorRGBA8 inline RGBA8_MidGrey()          { return RGBA8(128, 128, 128, 255); }
	static ColorRGBA8 inline RGBA8_LightGrey()        { return RGBA8(204, 204, 204, 255); }
	static ColorRGBA8 inline RGBA8_Red()              { return RGBA8(255, 0, 0, 255); }
	static ColorRGBA8 inline RGBA8_Green()            { return RGBA8(0, 255, 0, 255); }
	static ColorRGBA8 inline RGBA8_Blue()             { return RGBA8(0, 0, 255, 255); }
	static ColorRGBA8 inline RGBA8_Yellow()           { return RGBA8(0, 255, 255, 255); }
	static ColorRGBA8 inline RGBA8_Orange()           { return RGBA8(204, 77, 51, 255); }
	static ColorRGBA8 inline RGBA8_Magenta()          { return RGBA8(255, 0, 255, 255); }
	static ColorRGBA8 inline RGBA8_LightBlue()        { return RGBA8(51, 77, 179, 255); }

	// Helper functions 
	inline bool operator==(const ColorRGBA8& left, const ColorRGBA8& other)
	{
		return (left.r == other.r && left.g == other.g && left.b == other.b && left.a == other.a);
	} 

	inline ColorRGBA8 SetOpacity(ColorRGBA8& left, float a)
	{
		return RGBA8(left.r, left.g, left.b, (int)(a * 255));
	}

	// Strict POD struct ColoRGBA16
	struct ColorRGBA16
	{ 
		float r;
		float g;
		float b;
		float a; 
	};

	// Initializer
	inline ColorRGBA16 RGBA16(float r, float g, float b, float a)
	{
		ColorRGBA16 color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;

		return color;
	}

	// Initializer
	inline ColorRGBA16 RGBA16(float val)
	{
		ColorRGBA16 color;
		color.r = val;
		color.b = val;
		color.g = val;
		color.a = 1.0f;

		return color;
	}
	
	// Initializer
	inline ColorRGBA16 RGBA16(float val, float a)
	{
		ColorRGBA16 color;
		color.r = val;
		color.b = val;
		color.g = val;
		color.a = a;

		return color;
	}
	
	// Predefined Color Types
	static ColorRGBA16 inline RGBA16_White()                { return RGBA16(1.0f); }
	static ColorRGBA16 inline RGBA16_Black()                { return RGBA16(0.0f, 0.0f, 0.0f, 1.0f); }
	static ColorRGBA16 inline RGBA16_LightGrey()		    { return RGBA16(0.8f, 0.8f, 0.8f, 1.0f); }
	static ColorRGBA16 inline RGBA16_MidGrey()              { return RGBA16(0.5f, 0.5f, 0.5f, 1.0f); } 
	static ColorRGBA16 inline RGBA16_DarkGrey()             { return RGBA16(0.1f, 0.1f, 0.1f, 1.0f); }
	static ColorRGBA16 inline RGBA16_Red()			        { return RGBA16(1.0f, 0.0f, 0.0f, 1.0f); }
	static ColorRGBA16 inline RGBA16_Green()				{ return RGBA16(0.0f, 1.0f, 0.0f, 1.0f); }
	static ColorRGBA16 inline RGBA16_Blue()					{ return RGBA16(0.0f, 0.0f, 1.0f, 1.0f); }
	static ColorRGBA16 inline RGBA16_Yellow()               { return RGBA16(0.0f, 1.0f, 1.0f, 1.0f); }
	static ColorRGBA16 inline RGBA16_Magenta()              { return RGBA16(1.0f, 0.0f, 1.0f, 1.0f); } 
	static ColorRGBA16 inline RGBA16_Orange()	     	    { return RGBA16(0.8f, 0.3f, 0.2f, 1.0f); }

	static ColorRGBA16 inline RGBA16_LightPurple() 			{ return RGBA16(0.24f, 0.22f, 0.35f, 1.0f); }
	static ColorRGBA16 inline RGBA16_LimeGreen() 			{ return RGBA16(0.49f, 1.0f, 0.51f, 1.0f); }
	static ColorRGBA16 inline RGBA16_BurntOrange()	     	{ return RGBA16(0.9f, 0.5f, 0.2f, 1.0f); }
	static ColorRGBA16 inline RGBA16_SkyBlue()              { return RGBA16(0.2f, 0.3f, 0.8f, 1.0f); } 
	static ColorRGBA16 inline RGBA16_ZombieGreen()          { return RGBA16(0.33f, 0.77f, 0.44f, 1.0f); }
	static ColorRGBA16 inline RGBA16_DullMagenta()          { return RGBA16(0.77f, 0.33f, 0.66f, 1.0f); }

	// Helper functions
	inline ColorRGBA16 SetOpacity(const ColorRGBA16& color, float a)   
	{ 
		return RGBA16(color.r, color.g, color.b, a); 
	}

}}











#endif