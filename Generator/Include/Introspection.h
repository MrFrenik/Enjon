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

enum class MetaClassType
{
	Object, 
	Application,
	Component
};

enum class PropertyType
{
	Object,
	Bool,
	ColorRGBA32,
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
	HashMap,
	Vec2,
	Vec3,
	Vec4,
	Mat4,
	Quat,
	Enum,
	UUID,
	Transform,
	AssetHandle,
	EntityHandle
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

struct PropertyTraits
{
	bool IsPointer = false;
	bool IsEditable = false;
	float UIMin = 0.0f;
	float UIMax = 0.0f;
};

class Property
{
	friend Introspection;

	public:
		Property( ) = default;
		~Property( ) = default;

	protected:

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
		PropertyType mType;
		std::string mTypeRaw;
		std::string mTypeAppend = "";
		std::string mName; 
		PropertyFlags mFlags = PropertyFlags::None;
		std::unordered_set< std::string > mPropertyTraits;
		PropertyTraits mTraits;

		static PropertyTypeMap mPropertyTypeMap;
		static PropertyTypeAsStringMap mPropertyTypeStringMap;
};

struct FunctionSignature
{
	std::string mFunctionName;
	std::string mRetType;
	std::vector< std::string > mParameterList;
};

class Function
{ 
	friend Introspection;

	public:
		Function( ) = default;
		~Function( ) = default;

	public:
		FunctionSignature mSignature;
}; 

enum class ArraySizeType
{
	Fixed,
	Dynamic
};

class ArrayProperty : public Property
{
	friend Introspection;
	public:		
		ArrayProperty( )
		{
			mType = PropertyType::Array;
		}

		~ArrayProperty( ) = default;

	public:
		std::string mSizeString;
		ArraySizeType mArraySizeType;
		std::string mPropertyTypeRaw;
		PropertyType mArrayPropertyType;
};

class HashMapProperty : public Property
{
	friend Introspection;
	public:		
		HashMapProperty( )
		{
			mType = PropertyType::HashMap;
		}

		~HashMapProperty( ) = default;

	public:
		std::string mKeyPropertyTypeRaw;
		PropertyType mKeyPropertyType;
		std::string mValuePropertyTypeRaw;
		PropertyType mValuePropertyType; 
};

typedef std::unordered_map< std::string, Property* > PropertyTable ;
typedef std::unordered_map< std::string, Function > FunctionTable ;
typedef std::vector< std::string > NamespaceQualifiers;

struct ClassMarkupTraits
{ 
	void AddNamespaceQualifier( const std::string& ns )
	{
		mNamespaceQualifiers.push_back( ns );
	}

	void Construct( bool enabled )
	{
		mConstruct = enabled;
	}

	NamespaceQualifiers mNamespaceQualifiers; 
	bool mConstruct = false;
};

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

		bool HasFunction( const std::string& name );
		
		Function* GetFunction( const std::string& name );

		std::string GetQualifiedName( )
		{
			std::string className = "";
			for ( auto& ns : mTraits.mNamespaceQualifiers )
			{
				className += ns + "::"; 
			} 
			return ( className + mName ); 
		}

	protected:

		static void PushScope( )
		{
			mScopeCount++;
		}

		static void PopScope( )
		{
			mScopeCount = Max( mScopeCount - 1, (u32)0 );
		} 

		void AddArray( const ArrayProperty& arr );

		void AddProperty( Property* prop );

		void AddFunction( const Function& func );

	public:
		PropertyTable mProperties;
		FunctionTable mFunctions;
		ClassMarkupTraits mTraits;
		std::string mName; 
		std::string mFilePath;
		static u32 mScopeCount;
		std::string mParent = "";
		u32 mObjectTypeId = 0;
		bool mIsApplication = false;
		MetaClassType mMetaClassType = MetaClassType::Object;
}; 


struct EnumElement
{
	std::string mElementName;
	s32 mValue;
};
 

class Enum
{
	friend Introspection;
	public:
		Enum( ) = default;
		~Enum( ) = default; 
 
	public:
		std::string mName;
		std::vector< EnumElement > mElements;
};

struct ReflectionConfig
{
	void CollectFiles( Lexer* lexer );

	std::string mRootPath; 
	std::string mEnginePath;
	std::string mConfigFilePath;
	std::string mOutputDirectory;
	std::string mLinkedDirectory;
	std::string mProjectName; 
	std::vector< std::string > mFilesToParse;
	std::vector< std::string > mAdditionalIncludes;
	bool mIsApplication = false;
}; 

class Introspection
{
	public:
		Introspection( ); 

		~Introspection( );

		void Initialize( );

		void InitPropertyMap( );
 
		void Parse( Lexer* lexer );

		void ParseClassBody( Lexer* lexer, Class* cls );

		void ParseClass( Lexer* lexer );

		void ParseClassTraits( Lexer*, ClassMarkupTraits* traits );
		
		void ParseClassMembers( Lexer* lexer, Class* cls );

		void ParseProperty( Lexer* lexer, Class* cls );

		void ParseFunction( Lexer* lexer, Class* cls );

		PropertyType GetPropertyType( Lexer* lexer );

		bool IsPropertyArrayType( Lexer* lexer );

		bool IsPropertyHashMapType( Lexer* lexer );

		void ParseEnum( Lexer* lexer );

		bool EnumExists( const std::string& enumName ); 

		void RemoveClass( const std::string& className );

		const Enum* AddEnum( const std::string& enumName );

		void RemoveEnum( const std::string& enumName );

		const Enum* GetEnum( const std::string& name );
		
		bool ClassExists( const std::string& className );

		const Class* AddClass( const std::string& className );

		const Class* GetClass( const std::string& name );

		const Class* FindApplicationClass( );

		void Compile( const ReflectionConfig& config );

		void Link( const ReflectionConfig& config );

		PropertyType GetTypeFromString( const std::string& str );
		std::string GetTypeAsString( PropertyType type );

		void SetTypeID( const u32& typeId );

	private:
		std::string OutputLinkedHeader( const ReflectionConfig& config );

	private:
		std::unordered_map< std::string, Class > mClasses;
		std::unordered_map< std::string, Enum > mEnums;
		PropertyTypeMap mPropertyTypeMap;
		PropertyTypeAsStringMap mPropertyTypeStringMap;
		u32 mLastObjectTypeId = 0;
};

#endif