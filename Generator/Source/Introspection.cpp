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
	mPropertyTypeMap[ "float" ]			= PropertyType::F32;
	mPropertyTypeMap[ "f32" ]			= PropertyType::F32;
	mPropertyTypeMap[ "f64" ]			= PropertyType::F64;
	mPropertyTypeMap[ "double" ]		= PropertyType::F64;
	mPropertyTypeMap[ "u8" ]			= PropertyType::S8;
	mPropertyTypeMap[ "uint8_t" ]		= PropertyType::U8;
	mPropertyTypeMap[ "u16" ]			= PropertyType::U16;
	mPropertyTypeMap[ "uint16_t" ]		= PropertyType::U16;
	mPropertyTypeMap[ "u32" ]			= PropertyType::U32;
	mPropertyTypeMap[ "uint32_t" ]		= PropertyType::U32;
	mPropertyTypeMap[ "u64" ]			= PropertyType::U64;
	mPropertyTypeMap[ "uint64_t" ]		= PropertyType::U64;
	mPropertyTypeMap[ "s32" ]			= PropertyType::S32;
	mPropertyTypeMap[ "int32_t" ]		= PropertyType::S32;
	mPropertyTypeMap[ "s64" ]			= PropertyType::S64;
	mPropertyTypeMap[ "int64_t" ]		= PropertyType::S64;
	mPropertyTypeMap[ "Vec2" ]			= PropertyType::Vec2;
	mPropertyTypeMap[ "Vec3" ]			= PropertyType::Vec3;
	mPropertyTypeMap[ "Vec4" ]			= PropertyType::Vec4;
	mPropertyTypeMap[ "ColorRGBA16" ]	= PropertyType::ColorRGBA16;
	mPropertyTypeMap[ "UUID" ]			= PropertyType::UUID;
	mPropertyTypeMap[ "String" ]		= PropertyType::String;

	// Init property type as string
	PROP_TO_STRING( F32 )
	PROP_TO_STRING( F64 )
	PROP_TO_STRING( ColorRGBA16 )
	PROP_TO_STRING( U8 )
	PROP_TO_STRING( U16 )
	PROP_TO_STRING( U32 )
	PROP_TO_STRING( U64 )
	PROP_TO_STRING( S8 )
	PROP_TO_STRING( S16 )
	PROP_TO_STRING( S32 ) 
	PROP_TO_STRING( S64 )
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

			case TokenType::Token_OpenBrace:
			{
				Class::PushScope( );
			}

			case TokenType::Token_CloseBrace:
			{
				Class::PopScope( );
			}

			case TokenType::Token_Identifier:
			{ 
				// Parse class when object identifier is found
				if ( token.Equals( "ENJON_CLASS" ) )
				{
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

void Introspection::RemoveClass( const std::string& className )
{
	mClasses.erase( className );
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

	// TODO(): Grab special class traits here

	// Continue to close paren
	if ( !lexer->ContinueTo( TokenType::Token_CloseParen ) )
	{
		return;
	}

	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Look for class keyword
	if ( !lexer->GetCurrentToken( ).Equals( "class" ) )
	{
		return;
	}

	// Now need to grab class name
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Get class token
	Token classToken = lexer->GetCurrentToken( );

	// Get class name and store for class creation
	std::string className = classToken.ToString( );

	// Get new class created
	Class* cls = const_cast< Class* >( AddClass( className ) );

	// Set contents path of class ( include directory )
	cls->mFilePath = lexer->GetContentsPath( );

	// Find super class
	if ( lexer->PeekAtNextToken( ).IsType( TokenType::Token_Colon ) )
	{
		// Grab the colon
		Token colToken = lexer->GetNextToken( );

		// Peek at next token to see if is public keyword
		if ( lexer->PeekAtNextToken( ).Equals( "public" ) || lexer->PeekAtNextToken().Equals( "private" ) )
		{
			// Grab public/private keyword
			lexer->GetNextToken( );

			if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
			{
				return;
			}

			// Should be on the super class now
			Token superToken = lexer->GetCurrentToken( );

			cls->mParent = superToken.ToString( );
		}
	} 

	// Continue until open brace; if none hit, remove class
	if ( !lexer->ContinueTo( TokenType::Token_OpenBrace ) )
	{
		RemoveClass( className );

		return;
	} 

	// Need to push scope now, since open brace has been found
	Class::PushScope( ); 

	// Continue to class body tag, if not found then remove class and return
	if ( !lexer->ContinueToIdentifier( "ENJON_CLASS_BODY" ) )
	{
		// Remove class
		RemoveClass( className );

		return;
	}

	// Parse remainder of class body
	ParseClassBody( lexer, cls );

	// Pop scope from class
	Class::PopScope( );
}

//=================================================================================================
		
void Introspection::ParseClassBody( Lexer* lexer, Class* cls )
{
	// Grab next token and make sure is parentheses
	if ( !lexer->RequireToken( TokenType::Token_OpenParen, true ) )
	{
		RemoveClass( cls->mName );

		return;
	} 

	// Continue to last paren
	if ( !lexer->ContinueTo( TokenType::Token_CloseParen ) ) 
	{
		RemoveClass( cls->mName );

		return;
	}

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
	// New property to be filled out
	Property prop;

	// Grab next token and make sure is parentheses
	if ( !lexer->RequireToken( TokenType::Token_OpenParen, true ) )
	{
		return;
	}

	// Grab all inner property traits until close paren is hit ( or end of stream in case of error )
	{
		Token curToken = lexer->GetCurrentToken( );
		while ( !curToken.IsType( TokenType::Token_CloseParen ) && !curToken.IsType( TokenType::Token_EndOfStream ) )
		{
			// Get next token
			curToken = lexer->GetNextToken( );

			// If identifier, then push back intro property traits
			if ( curToken.IsType( TokenType::Token_Identifier ) )
			{
				// Push back trait
				prop.AddTrait( curToken.ToString( ) );
			}
		}
	} 

	// Need to strip away all namespaces from property
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Consume all namespace qualifiers 
	{
		Token curToken = lexer->GetCurrentToken( );
		Token nextToken = lexer->PeekAtNextToken( );
		while ( curToken.IsType( TokenType::Token_Identifier ) && nextToken.IsType( TokenType::Token_DoubleColon ) )
		{
			// Set to next token
			curToken = lexer->GetNextToken( );
			// This gets next token
			curToken = lexer->GetNextToken( ); 
			// Peek at next token
			nextToken = lexer->PeekAtNextToken( );
		}
	}

	// Get property type of current token
	if ( !lexer->RequireToken( TokenType::Token_Identifier ) )
	{
		return; 
	}
		
	// Grab current token
	Token curToken = lexer->GetCurrentToken( );

	// Get property type from identifier token string
	//prop.mType = Property::GetTypeFromString( curToken.ToString( ) );
	prop.mType = curToken.ToString( );

	// TODO(): Pointer types / Const references
		
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
			
			// Get parent
			Class* parentCls = nullptr;
			if ( ClassExists( c.second.mParent ) )
			{
				parentCls = const_cast< Class* >( GetClass( c.second.mParent ) );
			}

			// Copy over all properties into single map
			PropertyTable properties = c.second.mProperties;
			if ( parentCls )
			{
				for ( auto& p : parentCls->mProperties )
				{
					properties[ p.first ] = p.second;
				}
			}

			// Construct meta class function
			code += OutputLine( "// " + c.second.mName );
			code += OutputLine( "template <>" );
			code += OutputLine( "MetaClass* Object::ConstructMetaClass< " + c.second.mName + " >( )" );
			code += OutputLine( "{" );
			code += OutputTabbedLine( "MetaClass* cls = new MetaClass( );\n" ); 

			// Construct properties
			code += OutputTabbedLine( "// Construct properties" );
			code += OutputTabbedLine( "cls->mPropertyCount = " + std::to_string( properties.size( ) ) + ";" ); 

			// Iterate through properties and output code
			u32 index = 0;
			if ( !properties.empty( ) )
			{
				code += OutputTabbedLine( "cls->mProperties.resize( cls->mPropertyCount );" );
				code += OutputTabbedLine( "Enjon::MetaProperty props[] = " );
				code += OutputTabbedLine( "{" );

				for ( auto& prop : properties )
				{
					// Get property as string
					auto metaProp = Property::GetTypeFromString( prop.second.mType );
					std::string metaPropStr = Property::GetTypeAsString( metaProp ); 

					// Look for traits to fill out
					std::string traits = "MetaPropertyTraits( "; 
					traits += prop.second.HasTrait( "Editable" ) ? "true" : "false";
					traits += " )"; 

					// Get property name
					std::string pn = prop.second.mName;

					// Get class name
					std::string cn = c.second.mName;

					// Get property index
					std::string pi = std::to_string( index++ );

					// Get end character
					std::string endChar = index <= properties.size( ) - 1 ? "," : "";

					// Output line
					code += OutputTabbedLine( "\tEnjon::MetaProperty( MetaPropertyType::" + metaPropStr + ", \"" + pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + " )" + endChar ); 
				} 

				// End property table
				code += OutputTabbedLine( "};\n" );
	 
				// Assign properties
				code += OutputTabbedLine( "// Assign properties to class" ); 
				code += OutputTabbedLine( "cls->mProperties = Enjon::PropertyTable( props, props + cls->mPropertyCount );" ); 
			}

			// Return statement
			code += OutputLine( "\n\treturn cls;" );
			code += OutputLine( "}" ); 

			// Write to file
			f.write( code.c_str( ), code.length( ) );
		} 
	} 
}
		
std::string Introspection::OutputLinkedHeader( )
{
	std::string code = "";

	code += OutputLine( "// @file Enjon_Generated.cpp" );
	code += OutputLine( "// Copyright 2016-2017 John Jackson. All Rights Reserved." );
	code += OutputLine( "// This file has been generated. All modifications will be lost." );
	code += OutputLine( "" );

	return code;
}

//=================================================================================================

void Introspection::Link( const ReflectionConfig& config )
{
	// Open link file to write to
	std::string enginePath = config.mEnjonRootPath + "/Include/Engine.h";
	std::string typesPath = config.mEnjonRootPath + "/Include/System/Types.h";
	std::string definesPath = config.mEnjonRootPath + "/Include/Defines.h";
	std::string linkFilePath = config.mLinkedDirectory + "/" + "Enjon_Generated.cpp"; 
	std::ofstream f( linkFilePath ); 

	// Code to write to file
	std::string code = "";

	// Write header for Linked file
	code += OutputLinkedHeader( );

	// Output include diretories
	for ( auto& c : mClasses )
	{
		code += OutputLine( "#include \"" + c.second.mFilePath + "\"" );
	}

	// Output engine/types path include
	code += OutputLine( "#include \"" + enginePath + "\"" );
	code += OutputLine( "#include \"" + typesPath + "\"" );
	code += OutputLine( "#include \"" + definesPath + "\"" );

	// Output namespace
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














