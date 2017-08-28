#pragma once
#ifndef GENERATOR_INTROSPECTION_H
#define GENERATOR_INTROSPECTION_H

#include "Utils.h"

// Forward Declarations 
class Lexer;

class Class
{

};

class Struct
{

}; 

class Type
{

};

enum class PropertyType
{
	Float,
	UnsignedInteger,
	Integer,
	String,
	Double,
	Vec2,
	Vec3,
	Vec4,
	Mat4,
	Quat,
	Enum
};

enum PropertyFlags : u32
{
	None			= 0x00,
	IsPointer		= 0x01,
	IsDoublePointer = 0x02
};

inline PropertyFlags operator|( PropertyFlags a, PropertyFlags b )
{
	return static_cast<PropertyFlags>( static_cast<u32>( a ) | static_cast<u32>( b ) );
}

inline PropertyFlags operator&( PropertyFlags a, PropertyFlags b )
{
	return static_cast<PropertyFlags>( static_cast<u32>( a ) & static_cast<u32>( b ) );
}

inline PropertyFlags operator^( PropertyFlags a, PropertyFlags b )
{
	return static_cast<PropertyFlags>( static_cast<u32>( a ) ^ static_cast<u32>( b ) );
}

inline void operator^=( PropertyFlags& a, PropertyFlags b )
{
	a = a ^ b;
}

inline void operator|=( PropertyFlags& a, PropertyFlags b )
{
	a = a | b;
}

inline void operator&=( PropertyFlags& a, PropertyFlags b )
{
	a = a & b;
}

class Property
{
	public:
		Property( ) {}
		~Property( ) {} 

	public:
		PropertyType mType;
		const char* mName; 
		PropertyFlags mFlags = PropertyFlags::None;
};

class Function
{

}; 


class Introspection
{
	public:
		Introspection( ); 

		~Introspection( );

		void Parse( Lexer* lexer );

	private:
};

#endif