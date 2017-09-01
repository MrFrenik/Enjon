#include "Token.h"
#include "Utils.h" 
#include "Lexer.h"
#include "Introspection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <unordered_map>

// Possible implementation of meta class and generated fields for reflection

#define OBJECT( type )\
friend Object;\
public:\
	virtual u32 GetTypeId() const override { return Object::GetTypeId< type >(); }		\
	virtual const char* GetTypeName() const override { return #type; }\
	virtual const MetaClass* type::Class( ) override\
	{\
		const MetaClass* cls = mGlobalMetaRegistry.Get< type >( );\
		if ( !cls )\
		{\
			cls = mGlobalMetaRegistry.RegisterMetaClass< type >( );\
		}\
		return cls;\
	}

typedef uint32_t u32;
typedef int32_t s32;
typedef float f32; 
typedef char u8;

enum class MetaPropertyType
{
	Unknown,
	Bool,
	ColorRGBA16,
	Float_32,
	Float_64,
	Uint_8,
	Uint_16,
	Uint_32,
	Uint_64,
	Int_8,
	Int_16,
	Int_32,
	Int_64,
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

class MetaClass;
class Object;
class MetaProperty
{ 
	friend MetaClass;
	friend Object;
	public:
		MetaProperty( ) = default;
		MetaProperty( MetaPropertyType type, const std::string& name, u32 offset )
		{
			mType = type;
			mName = name;
			mOffset = offset;
		}
		~MetaProperty( )
		{ 
		} 

		std::string GetName( )
		{
			return mName;
		}

		MetaPropertyType GetType( )
		{
			return mType;
		}

	protected:
		MetaPropertyType mType;
		std::string mName;
		u32 mOffset;
};

class MetaFunction
{ 
	public:
};

// Class has funtion

class MetaClass
{ 
	friend Object;

	public:
		MetaClass( ) = default; 
		~MetaClass( )
		{ 
		}

		u32 PropertyCount( ) const
		{
			return ( u32 )mProperties.size( );
		}

		u32 FunctionCount( )
		{
			return ( u32 )mFunctions.size( );
		} 

		bool PropertyExists( const std::string& propertyName )
		{
			return ( mProperties.find( propertyName ) != mProperties.end( ) );
		}

		const MetaProperty* GetProperty( const std::string& propertyName )
		{
			if ( PropertyExists( propertyName ) )
			{
				return &mProperties[ propertyName ];
			}
			return nullptr;
		}

		template < typename T >
		void GetValue( const Object* obj, const MetaProperty* property, T* value )
		{
			if ( PropertyExists( property->mName ) )
			{
				void* member_ptr = ( ( ( u8* )&( *obj ) + property->mOffset ) );
				*value = *( T* )member_ptr;
			} 
		}
		
		template < typename T >
		void GetValue( const Object* obj, const std::string& propertyName, T* value )
		{
			if ( PropertyExists( propertyName ) )
			{
				MetaProperty* prop = &mProperties[ propertyName ];
				void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
				*value = *( T* )member_ptr;
			} 
		}
		
		template < typename T >
		void SetValue( const Object* obj, const std::string& propertyName, const T& value )
		{ 
			if ( PropertyExists( propertyName ) )
			{ 
				MetaProperty* prop = &mProperties[ propertyName ];
				void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) ); 
				*( T* )( member_ptr ) = value;
			}
		}
		
		template < typename T >
		void SetValue( const Object* obj, const MetaProperty* prop, const T& value )
		{ 
			if ( PropertyExists( prop->mName ) )
			{
				void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) ); 
				*( T* )( member_ptr ) = value; 
			}
		} 

	protected:	
		std::unordered_map< std::string, MetaProperty > mProperties;
		std::unordered_map< std::string, MetaFunction > mFunctions; 
		std::string mIncludeDirectory;
}; 

class MetaClassRegistry
{
	public:
		MetaClassRegistry( )
		{ 
		}

		~MetaClassRegistry( )
		{ 
		}

		template <typename T>
		MetaClass* RegisterMetaClass( )
		{ 
			// Must inherit from object to be able to registered
			static_assert( std::is_base_of<Object, T>::value, "RegisterMetaClass:: T must inherit from Object." );

			// Get id of object
			u32 id = Object::GetTypeId<T>( );

			// If available, then return
			if ( HasMetaClass< T >( ) )
			{
				return mRegistry[ id ];
			}

			// Otherwise construct it and return
			 MetaClass* cls = Object::ConstructMetaClass< T >( );

			mRegistry[ id ] = cls; 
			return cls;
		}
		
		template < typename T >
		bool HasMetaClass( )
		{
			return ( mRegistry.find( Object::GetTypeId< T >( ) ) != mRegistry.end( ) );
		}

		template < typename T >
		const MetaClass* Get( )
		{
			return HasMetaClass< T >( ) ? mRegistry[ Object::GetTypeId< T >( ) ] : nullptr;
		}

	private:
		std::unordered_map< u32, MetaClass* > mRegistry;
};

class Object
{ 
	friend MetaClassRegistry;

	public:
		Object()
		{ 
		}

		/**
		*@brief
		*/
		template <typename T>
		static u32 GetTypeId( ) noexcept
		{
			static_assert( std::is_base_of<Object, T>::value, "GetTypeId:: T must inherit from Object." );

			static u32 typeId { GetUniqueTypeId( ) };
			return typeId;
		}
		
		/**
		*@brief
		*/
		template <typename T>
		T* Cast( )
		{
			static_assert( std::is_base_of<Object, T>::value, "Cast:: T must inherit from Object." );

			return static_cast<T*>( this );
		}
		
		virtual const MetaClass* Class( ) = 0; 

		virtual u32 GetTypeId( ) const = 0;

		virtual const char* GetTypeName( ) const = 0;

	protected:
		template <typename T>
		static MetaClass* ConstructMetaClass( );

	private:
		/**
		*@brief
		*/
		static u32 GetUniqueTypeId( ) noexcept
		{
			static u32 lastId { 1 };
			return lastId++;
		}

};

// Global registry for meta class information
MetaClassRegistry mGlobalMetaRegistry;

class Thing : public Object 
{
	OBJECT( Thing )

	public:
		Thing( )
		{ 
		}
		
		Thing( f32 f, u32 u, bool b, const std::string& string )
			: mFloatProperty( f ), mUintProperty( u ), mBoolProperty( b ), mStringProperty( string )
		{ 
		}

		~Thing( )
		{
		}

	private:
		float mFloatProperty;
		u32 mUintProperty;
		bool mBoolProperty;
		std::string mStringProperty;
}; 

class OtherThing : public Object 
{
	OBJECT( OtherThing )

	public:
		OtherThing( )
		{ 
		}
		
		OtherThing( f32 f, u32 u, bool b, const std::string& string )
			: mOtherFloatProperty( f ), mOtherUintProperty( u ), mOtherBoolProperty( b ), mOtherStringProperty( string )
		{ 
		}

		~OtherThing( )
		{
		}

	private:
		float mOtherFloatProperty;
		u32 mOtherUintProperty;
		bool mOtherBoolProperty;
		std::string mOtherStringProperty;
}; 
 

// GENERATION WOULD BEGIN HERE ////////////////////

// Template specialization for meta class construction
template <>
MetaClass* Object::ConstructMetaClass<Thing>( )
{
	MetaClass* cls = new MetaClass( );

	// Construct properties
	cls->mProperties[ "mFloatProperty" ]	= MetaProperty( MetaPropertyType::Float_32, "mFloatProperty", ( u32 )&( ( Thing* )0 )->mFloatProperty );
	cls->mProperties[ "mUintProperty" ]		= MetaProperty( MetaPropertyType::Uint_32, "mUintProperty", ( u32 )&( ( Thing* )0 )->mUintProperty );
	cls->mProperties[ "mBoolProperty" ]		= MetaProperty( MetaPropertyType::Bool, "mBoolProperty", ( u32 )&( ( Thing* )0 )->mBoolProperty );
	cls->mProperties[ "mStringProperty" ]	= MetaProperty( MetaPropertyType::String, "mStringProperty", ( u32 )&( ( Thing* )0 )->mStringProperty );

	return cls;
}

// Template specialization for meta class construction
template <>
MetaClass* Object::ConstructMetaClass<OtherThing>( )
{
	MetaClass* cls = new MetaClass( );

	// Construct properties
	cls->mProperties[ "mOtherFloatProperty" ]	= MetaProperty( MetaPropertyType::Float_32, "mOtherFloatProperty", ( u32 )&( ( OtherThing* )0 )->mOtherFloatProperty );
	cls->mProperties[ "mOtherUintProperty" ]	= MetaProperty( MetaPropertyType::Uint_32, "mOtherUintProperty", ( u32 )&( ( OtherThing* )0 )->mOtherUintProperty );
	cls->mProperties[ "mOtherBoolProperty" ]	= MetaProperty( MetaPropertyType::Bool, "mOtherBoolProperty", ( u32 )&( ( OtherThing* )0 )->mOtherBoolProperty );
	cls->mProperties[ "mOtherStringProperty" ]	= MetaProperty( MetaPropertyType::String, "mOtherStringProperty", ( u32 )&( ( OtherThing* )0 )->mOtherStringProperty );

	return cls;
}

// END GENERATION //////////////////////////// 

int main( int argc, char** argv )
{ 
	ReflectionConfig mConfig; 
	Introspection mIntrospection;
	mIntrospection.Initialize( );

	Thing thing( -10.5f, 6, true, "test" );
	Thing thing2( 145.543f, 5, true, "thing" );
	OtherThing otherThing( -100.34, 34, false, "otherThing" );

	MetaClass* otherCls = const_cast< MetaClass* >( otherThing.Class( ) );
	{
		const MetaProperty* floatProp = otherCls->GetProperty( "mOtherFloatProperty" );
		f32 value = 0.0f;
		if ( floatProp )
		{
			otherCls->GetValue( &otherThing, floatProp, &value ); 
			f32 newValue = 12.34f; 
			otherCls->SetValue( &otherThing, "mOtherFloatProperty", newValue );
		}
	}

	MetaClass* cls = const_cast< MetaClass* > ( thing.Class( ) );
	const MetaClass* thing2Class = thing2.Class( );

	const MetaProperty* floatProp = cls->GetProperty( "mFloatProperty" );
	f32 value = 0.0f;
	f32 value2 = 0.0f; 
	if ( floatProp )
	{
		cls->GetValue( &thing, "mFloatProperty", &value );
		cls->GetValue( &thing2, floatProp, &value2 );

		f32 newValue = 1.0f;
		f32 newValue2 = 13.43f;

		cls->SetValue( &thing, floatProp, newValue );
		cls->SetValue( &thing2, "mFloatProperty", newValue2 );
	}

	const MetaProperty* uintProp = cls->GetProperty( "mUintProperty" );
	u32 uValue = 0;
	u32 uValue2 = 0;
	if ( uintProp )
	{
		cls->GetValue( &thing, uintProp, &uValue );
		cls->GetValue( &thing2, uintProp, &uValue2 ); 
	}

	const MetaProperty* boolProp = cls->GetProperty( "mBoolProperty" );
	bool bValue = false;
	bool bValue2 = false;
	if ( boolProp )
	{
		cls->GetValue( &thing, boolProp, &bValue );
		cls->GetValue( &thing2, boolProp, &bValue2 );
	} 

	const MetaProperty* stringProp = cls->GetProperty( "mStringProperty" );
	std::string strValue = "";
	std::string strValue2 = "";
	if ( stringProp )
	{
		std::string newValue = "hooah";
		std::string newValue2 = "nuts";

		cls->GetValue( &thing, "mStringProperty", &strValue );
		cls->GetValue( &thing2, stringProp, &strValue2 );

		cls->SetValue( &thing, "mStringProperty", newValue );
		cls->SetValue( &thing2, stringProp, newValue2 );
	}

	// Parse arguments and place into config
	for ( s32 i = 0; i < argc; ++i )
	{
		std::string arg = std::string( argv[i] ); 

		// Set root path
		if ( arg.compare( "--enjon-path" ) == 0 && (i + 1) < argc )
		{
			mConfig.mEnjonRootPath = FindReplaceMetaTag( String( argv[i + 1] ), "/Generator/..", "" );
		}
		
		// Set root path
		if ( arg.compare( "--project-path" ) == 0 && (i + 1) < argc )
		{
			mConfig.mProjectPath = String( argv[i + 1] );
		}
	} 

	// Grab the config file
	mConfig.mConfigFilePath = mConfig.mEnjonRootPath + "/Generator/config.cfg"; 
	mConfig.mOutputDirectory = mConfig.mEnjonRootPath + "/Build/Generator/Intermediate";
	mConfig.mLinkedDirectory = mConfig.mEnjonRootPath + "/Build/Generator/Linked";

	std::string configFileContents = ReadFileIntoString( mConfig.mConfigFilePath.c_str( ) );

	// Create new lexer and assign config file
	Lexer* lexer = new Lexer( configFileContents ); 

	// Collect all necessary files for reflection
	mConfig.CollectFiles( lexer );

	// Iterate over collected files and parse
	for ( auto& f : mConfig.mFilesToParse )
	{ 
		std::vector< std::string > splits = SplitString( f, "/" );

		if ( splits.back( ).compare( "Object.h" ) == 0 )
		{
			continue;
		}

		// Get file contents
		std::string fileToParse = ReadFileIntoString( f.c_str( ) ); 

		// Set lexer to parse contents
		lexer->SetContents( fileToParse, f );

		// Parse file and collect information
		mIntrospection.Parse( lexer ); 

		std::cout << "Generating reflecton for " << splits.back() <<  "\n";
	} 

	// Write classes to file
	mIntrospection.Compile( mConfig ); 

	// Link all classes into one generated file
	mIntrospection.Link( mConfig ); 
}
 
//====================================================================================

void ReflectionConfig::CollectFiles( Lexer* lexer )
{
	// Parse file contents
	bool Parsing = true;
	while ( Parsing )
	{
		// Grab token from lexer
		Token token = lexer->GetNextToken( );

		// Switch on token type given
		switch ( token.mType )
		{
			// Start identifer 
			case TokenType::Token_Hash:
			{
				// Get next token
				Token nextToken = lexer->GetNextToken( );

				if ( nextToken.mType == TokenType::Token_Identifier )
				{ 
					if ( nextToken.Equals( "enjon_dir" ) )
					{
					}
					if ( nextToken.Equals( "files" ) )
					{
						// Advance next token - if is string, then is file name
						while ( lexer->RequireToken( TokenType::Token_String, true ) )
						{
							mFilesToParse.push_back( lexer->GetCurrentToken( ).ToString( ) ); 
						}
					} 
				}
			} break;

			case TokenType::Token_Unknown:
			{
				Parsing = false;
			}
			break;

			case TokenType::Token_EndOfStream:
			{
				Parsing = false;
			}
			break; 
		}
	}
}

