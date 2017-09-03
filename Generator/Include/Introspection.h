#pragma once
#ifndef GENERATOR_INTROSPECTION_H
#define GENERATOR_INTROSPECTION_H

#include "Utils.h" 

// Forward Declarations 
class Lexer;
class Introspection;
class Class;
class Struct;
class Type; 

enum class PropertyType
{
	Unknown,
	Bool,
	ColorRGBA16,
	F32,
	F64,
	U8,
	U16,
	U32,
	U64,
	S8,
	S16,
	S32,
	S64,
	String,
	Array,
	Vec2,
	Vec3,
	Vec4,
	Mat4,
	Quat,
	Enum,
	UUID
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

typedef std::unordered_map< std::string, PropertyType > PropertyTypeMap;
typedef std::unordered_map< PropertyType, std::string > PropertyTypeAsStringMap;

class Property
{
	friend Introspection;

	public:
		Property( ) {}
		~Property( ) {} 
 
		static PropertyType GetTypeFromString( const std::string& str );
		static std::string GetTypeAsString( PropertyType type );

	protected:
		static void InitPropertyMap( );

		bool HasTrait( const std::string& trait )
		{
			return ( mPropertyTraits.find( trait ) != mPropertyTraits.end( ) );
		}

		void AddTrait( const std::string& trait )
		{
			if ( !HasTrait( trait ) )
			{
				mPropertyTraits.insert( trait );
			}
		}

	public:
		std::string mType;
		std::string mName; 
		PropertyFlags mFlags = PropertyFlags::None;
		std::unordered_set< std::string > mPropertyTraits;

		static PropertyTypeMap mPropertyTypeMap;
		static PropertyTypeAsStringMap mPropertyTypeStringMap;
};

class Function
{ 
}; 

typedef std::unordered_map< std::string, Property > PropertyTable ;
typedef std::unordered_map< std::string, Function > FunctionTable ;

class Class
{
	friend Introspection;
	friend Lexer;

	public:
		Class( )
		{ 
		}
		
		Class( const std::string& name )
			: mName( name )
		{ 
		}

		~Class( )
		{ 
		}

		bool HasProperty( const std::string& name );

		Property* GetProperty( const std::string& name );

	protected:

		static void PushScope( )
		{
			mScopeCount++;
		}

		static void PopScope( )
		{
			mScopeCount = Max( mScopeCount - 1, (u32)0 );
		} 

		void AddProperty( const Property& prop );

	public:
		PropertyTable mProperties;
		FunctionTable mFunctions;
		std::string mName; 
		std::string mFilePath;
		static u32 mScopeCount;
		std::string mParent = "";
};

class Struct
{ 
}; 

class Type
{

};

struct ReflectionConfig
{
	void CollectFiles( Lexer* lexer );

	std::string mEnjonRootPath;
	std::string mProjectPath;
	std::string mConfigFilePath;
	std::string mOutputDirectory;
	std::string mLinkedDirectory;
	std::vector< std::string > mFilesToParse;
}; 

class Introspection
{
	public:
		Introspection( ); 

		~Introspection( );

		void Initialize( );
 
		void Parse( Lexer* lexer );

		void ParseClassBody( Lexer* lexer, Class* cls );

		void ParseClass( Lexer* lexer );
		
		void ParseClassMembers( Lexer* lexer, Class* cls );

		void ParseProperty( Lexer* lexer, Class* cls );

		void ParseFunction( Lexer* lexer, Class* cls );

		bool ClassExists( const std::string& className );

		void RemoveClass( const std::string& className );

		const Class* AddClass( const std::string& className );

		const Class* GetClass( const std::string& name );

		void Compile( const ReflectionConfig& config );

		void Link( const ReflectionConfig& config );

	private:
		std::string OutputLinkedHeader( );

	private:
		std::unordered_map< std::string, Class > mClasses;
};

#endif