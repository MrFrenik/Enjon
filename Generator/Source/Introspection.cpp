#include "Introspection.h"
#include "Token.h"
#include "Lexer.h"

#include <iostream>
#include <fstream>

// Static variables
u32 Class::mScopeCount = 0; 
PropertyTypeMap Property::mPropertyTypeMap;
PropertyTypeAsStringMap Property::mPropertyTypeStringMap;

//================================================================================================= 
		
std::string Property::GetTypeAsString( PropertyType type )
{
	if ( mPropertyTypeStringMap.find( type ) != mPropertyTypeStringMap.end( ) )
	{
		return mPropertyTypeStringMap[ type ];
	}

	return "Unknown";
}

//================================================================================================= 
		
PropertyType Property::GetTypeFromString( const std::string& str )
{
	// If found, then return
	if ( mPropertyTypeMap.find( str ) != mPropertyTypeMap.end( ) )
	{
		return mPropertyTypeMap[ str ];
	}

	return PropertyType::Unknown;
}

//=================================================================================================

#define PROP_TO_STRING( prop )\
mPropertyTypeStringMap[ PropertyType::prop ] = #prop; 

void Property::InitPropertyMap( )
{
	mPropertyTypeMap[ "bool" ]			= PropertyType::Bool;
	mPropertyTypeMap[ "float" ]			= PropertyType::Float_32;
	mPropertyTypeMap[ "f32" ]			= PropertyType::Float_32;
	mPropertyTypeMap[ "f64" ]			= PropertyType::Float_64;
	mPropertyTypeMap[ "double" ]		= PropertyType::Float_64;
	mPropertyTypeMap[ "u8" ]			= PropertyType::Uint_8;
	mPropertyTypeMap[ "uint8_t" ]		= PropertyType::Uint_8;
	mPropertyTypeMap[ "u16" ]			= PropertyType::Uint_16;
	mPropertyTypeMap[ "uint16_t" ]		= PropertyType::Uint_16;
	mPropertyTypeMap[ "u32" ]			= PropertyType::Uint_32;
	mPropertyTypeMap[ "uint32_t" ]		= PropertyType::Uint_32;
	mPropertyTypeMap[ "u64" ]			= PropertyType::Uint_64;
	mPropertyTypeMap[ "uint64_t" ]		= PropertyType::Uint_64;
	mPropertyTypeMap[ "s32" ]			= PropertyType::Int_32;
	mPropertyTypeMap[ "int32_t" ]		= PropertyType::Int_32;
	mPropertyTypeMap[ "s64" ]			= PropertyType::Int_64;
	mPropertyTypeMap[ "int64_t" ]		= PropertyType::Int_64;
	mPropertyTypeMap[ "Vec2" ]			= PropertyType::Vec2;
	mPropertyTypeMap[ "Vec3" ]			= PropertyType::Vec3;
	mPropertyTypeMap[ "Vec4" ]			= PropertyType::Vec4;
	mPropertyTypeMap[ "ColorRGBA16" ]	= PropertyType::ColorRGBA16;
	mPropertyTypeMap[ "UUID" ]			= PropertyType::UUID;

	// Init property type as string
	PROP_TO_STRING( Float_32 )
	PROP_TO_STRING( Float_64 )
	PROP_TO_STRING( ColorRGBA16 )
	PROP_TO_STRING( Uint_8 )
	PROP_TO_STRING( Uint_16 )
	PROP_TO_STRING( Uint_32 )
	PROP_TO_STRING( Uint_64 )
	PROP_TO_STRING( Int_8 )
	PROP_TO_STRING( Int_16 )
	PROP_TO_STRING( Int_32 ) 
	PROP_TO_STRING( Int_64 )
	PROP_TO_STRING( String )
	PROP_TO_STRING( Array )
	PROP_TO_STRING( Vec2 )
	PROP_TO_STRING( Vec3 )
	PROP_TO_STRING( Vec4 )
	PROP_TO_STRING( Mat4 )
	PROP_TO_STRING( Quat )
	PROP_TO_STRING( Enum ) 
	PROP_TO_STRING( UUID ) 
	PROP_TO_STRING( Bool ) 
}

//=================================================================================================
		
bool Class::HasProperty( const std::string& name )
{
	return ( mProperties.find( name ) != mProperties.end( ) );
}

//=================================================================================================

Property* Class::GetProperty( const std::string& name )
{
	if ( HasProperty( name ) )
	{
		return &mProperties[ name ];
	}

	return nullptr;
}

//=================================================================================================
		
void Class::AddProperty( const Property& prop )
{
	if ( !HasProperty( prop.mName ) )
	{
		mProperties[ prop.mName ] = prop;
	}
}

//================================================================================================= 

Introspection::Introspection( )
{ 
}

//=================================================================================================
		
Introspection::~Introspection( )
{ 
}

//=================================================================================================

void Introspection::Initialize( )
{
	// Set up property table
	Property::InitPropertyMap( );
}

//=================================================================================================

void Introspection::Parse( Lexer* lexer )
{
	// Parse file contents
	bool isParsing = true;
	while ( isParsing )
	{
		// Grab token from lexer
		Token token = lexer->GetNextToken( ); 

		// Switch on token type given
		switch ( token.mType )
		{
				// Start identifer 
			case TokenType::Token_Hash:
			{
			} break;

			case TokenType::Token_Identifier:
			{ 
				// Parse class when object identifier is found
				if ( token.Equals( "ENJON_OBJECT" ) )
				{
					// Begin class scope
					Class::PushScope( );

					// Parse the class
					ParseClass( lexer );
				}
			}
			break;

			case TokenType::Token_Unknown:
			{
				//Parsing = false;
			}
			break;

			case TokenType::Token_EndOfStream:
			{
				isParsing = false;
			}
			break;
		}
	}
}

//=================================================================================================
		
const Class* Introspection::GetClass( const std::string& name )
{
	if ( ClassExists( name ) )
	{
		return &mClasses[ name ];
	}

	return nullptr;
}

//=================================================================================================
		
bool Introspection::ClassExists( const std::string& className )
{
	return ( mClasses.find( className ) != mClasses.end( ) );
}

//=================================================================================================

const Class* Introspection::AddClass( const std::string& className )
{
	if ( !ClassExists( className ) )
	{
		mClasses[ className ] = Class( className );
	}

	return &mClasses[ className ];
}

//=================================================================================================
		
void Introspection::ParseClass( Lexer* lexer )
{
	// Grab next token and make sure is parentheses
	if ( !lexer->RequireToken( TokenType::Token_OpenParen, true ) )
	{
		return;
	}

	// Get class name
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Get class token
	Token classToken = lexer->GetCurrentToken( );

	// Get class name
	std::string className = classToken.ToString( );

	// Get new class created
	Class* cls = const_cast< Class* >( AddClass( className ) ); 

	// Set contents path of class ( include directory )
	cls->mFilePath = lexer->GetContentsPath( );

	// Continue to grab tokens until last paren is found
	lexer->ContinueTo( TokenType::Token_CloseParen );

	// Now need to parse all remaining members of class
	ParseClassMembers( lexer, cls );

}

//=================================================================================================

void Introspection::ParseClassMembers( Lexer* lexer, Class* cls )
{
	bool isParsing = true;
	while ( isParsing )
	{
		Token token = lexer->GetNextToken( );

		switch ( token.mType )
		{
			case TokenType::Token_Identifier:
			{
				// Parse class property
				if ( token.Equals( "ENJON_PROPERTY" ) )
				{
					ParseProperty( lexer, cls );
				}

				// Parse class function
				else if ( token.Equals( "ENJON_FUNCTION" ) )
				{
					ParseFunction( lexer, cls );
				}

			} break;

			case TokenType::Token_EndOfStream: 
			{
				isParsing = false; 
			} break;
		}
	}
}

//=================================================================================================

void Introspection::ParseProperty( Lexer* lexer, Class* cls )
{
	// Grab next token and make sure is parentheses
	if ( !lexer->RequireToken( TokenType::Token_OpenParen, true ) )
	{
		return;
	}
	
	// Continue to grab tokens until last paren is found
	lexer->ContinueTo( TokenType::Token_CloseParen );

	// Now need to create new property and add it to class 
	Property prop;

	// Get property type
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return; 
	}
		
	// Grab current token
	Token curToken = lexer->GetCurrentToken( );

	// Get property type from identifier token string
	//prop.mType = Property::GetTypeFromString( curToken.ToString( ) );
	prop.mType = curToken.ToString( );

	// TODO(): Pointer types 
		
	// Get property name
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Grab token for property name
	curToken = lexer->GetCurrentToken( ); 

	// Get name
	prop.mName = curToken.ToString( ); 

	// Add to class
	cls->AddProperty( prop ); 
}

//=================================================================================================

void Introspection::ParseFunction( Lexer* lexer, Class* cls )
{ 
}

//=================================================================================================

void Introspection::Compile( const ReflectionConfig& config )
{
	for ( auto& c : mClasses )
	{
		// Grab output path
		std::string outputPath = config.mOutputDirectory + "/" + c.second.mName + "_generated.gen"; 

		// Open file
		std::ofstream f( outputPath );
		if ( f )
		{
			// Build code
			std::string code; 

			// Construct meta class function
			code += OutputLine( "// " + c.second.mName );
			code += OutputLine( "template <>" );
			code += OutputLine( "MetaClass* Object::ConstructMetaClass< " + c.second.mName + " >( )" );
			code += OutputLine( "{" );
			code += OutputTabbedLine( "MetaClass* cls = new MetaClass( );\n" );
			code += OutputTabbedLine( "// Construct properties\n" );

			// Iterate through properties and output code
			for ( auto& prop : c.second.mProperties )
			{
				// Get property as string
				auto metaProp = Property::GetTypeFromString( prop.second.mType );
				std::string metaPropStr = Property::GetTypeAsString( metaProp ); 

				// Get property name
				std::string pn = prop.second.mName;

				// Get class name
				std::string cn = c.second.mName;

				// Output line
				code += OutputTabbedLine( "cls->mProperties[ \"" + pn + "\" ] = Enjon::MetaProperty( MetaPropertyType::" + metaPropStr + ", \"" + pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + " );" ); }

			// Return statement
			code += OutputLine( "\n\treturn cls;" );
			code += OutputLine( "}" ); 

			// Write to file
			f.write( code.c_str( ), code.length( ) );
		} 
	} 
}

//=================================================================================================

void Introspection::Link( const ReflectionConfig& config )
{
	// Open link file to write to
	std::string linkFilePath = config.mLinkedDirectory + "/" + "Enjon_Generated.cpp"; 
	std::ofstream f( linkFilePath ); 

	// Code to write to file
	std::string code = "";

	// Output include diretories
	for ( auto& c : mClasses )
	{
		code += OutputLine( "#include \"" + c.second.mFilePath + "\"" );
	}

	code += OutputLine( "\nusing namespace Enjon;\n" ); 

	// Iterate classes and grab compiled intermediate files
	for ( auto& c : mClasses )
	{
		// Grab output path
		std::string inputPath = config.mOutputDirectory + "/" + c.second.mName + "_generated.gen"; 

		// Open file
		std::string fileContents = ReadFileIntoString( inputPath.c_str( ) ); 

		// Append contents
		code += OutputLine( fileContents );
	} 

	// Output linked code
	if ( f )
	{
		f.write( code.c_str( ), code.length( ) );
	}
}














