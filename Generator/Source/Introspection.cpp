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

	return "Object";
}

//================================================================================================= 
		
PropertyType Property::GetTypeFromString( const std::string& str )
{
	// If found, then return
	if ( mPropertyTypeMap.find( str ) != mPropertyTypeMap.end( ) )
	{
		return mPropertyTypeMap[ str ];
	}

	return PropertyType::Object;
}

//================================================================================================= 

#define STRING_TO_PROP( str, prop )\
mPropertyTypeMap[ str ] = PropertyType::prop;\
mPropertyTypeStringMap[ PropertyType::prop ] = #prop;

void Property::InitPropertyMap( )
{
	STRING_TO_PROP( "bool", Bool )
	STRING_TO_PROP( "b8", Bool )
	STRING_TO_PROP( "float", F32 )
	STRING_TO_PROP( "f32", F32 )
	STRING_TO_PROP( "f64", F64 )
	STRING_TO_PROP( "double", F64 )
	STRING_TO_PROP( "u8", U8 )
	STRING_TO_PROP( "uint8_t", U8 )
	STRING_TO_PROP( "u16", U16 )
	STRING_TO_PROP( "uint16_t", U16 )
	STRING_TO_PROP( "u32", U32 )
	STRING_TO_PROP( "uint32_t", U32 )
	STRING_TO_PROP( "u64", U64 )
	STRING_TO_PROP( "uint64_t", U64 )
	STRING_TO_PROP( "s32", S32 )
	STRING_TO_PROP( "int32_t", S32 )
	STRING_TO_PROP( "s64", S64 )
	STRING_TO_PROP( "int64_t", S64 )
	STRING_TO_PROP( "Vec2", Vec2 )
	STRING_TO_PROP( "Vec3", Vec3 )
	STRING_TO_PROP( "Vec4", Vec4 )
	STRING_TO_PROP( "ColorRGBA16", ColorRGBA16 )
	STRING_TO_PROP( "UUID", UUID )
	STRING_TO_PROP( "String", String )
	STRING_TO_PROP( "Transform", Transform )
	STRING_TO_PROP( "Quaternion", Quat )
	STRING_TO_PROP( "AssetHandle", AssetHandle )
	STRING_TO_PROP( "EntityHandle", EntityHandle )
}

//=================================================================================================
		
bool Class::HasFunction( const std::string& name )
{
	return ( mFunctions.find( name ) != mFunctions.end( ) );
}

//=================================================================================================

Function* Class::GetFunction( const std::string& name )
{
	if ( HasFunction( name ) )
	{
		return &mFunctions[ name ];
	}

	return nullptr;
}

//=================================================================================================
		
void Class::AddFunction( const Function& func )
{
	if ( !HasFunction( func.mSignature.mFunctionName ) )
	{
		mFunctions[ func.mSignature.mFunctionName ] = func;
	}
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

			// Cache off parent string
			std::string parentString = superToken.ToString( );

			// Consume all namespace qualifiers 
			{
				Token curToken = lexer->GetCurrentToken( );
				Token nextToken = lexer->PeekAtNextToken( );
				while ( curToken.IsType( TokenType::Token_Identifier ) && nextToken.IsType( TokenType::Token_DoubleColon ) )
				{
					// Set to next token
					curToken = lexer->GetNextToken( );

					// Append to parent string
					parentString += curToken.ToString( );

					// This gets next token
					curToken = lexer->GetNextToken( );

					// Peek at next token
					nextToken = lexer->PeekAtNextToken( );
				}
			} 

			superToken = lexer->GetCurrentToken( );

			cls->mParent = parentString + superToken.ToString( );
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
				// Add editable trait
				if ( curToken.Equals( "Editable" ) )
				{
					prop.mTraits.IsEditable = true;
				}

				if ( curToken.Equals( "UIMin" ) || curToken.Equals( "UIMax" ) )
				{
					// Store token
					Token numToken = lexer->GetCurrentToken( );
					
					// Consume equal sign
					if ( !lexer->RequireToken( TokenType::Token_Equal, true ) )
					{
						return;
					} 

					// Get value
					if ( !lexer->RequireToken( TokenType::Token_Number, true ) )
					{
						return;
					}

					// Get value
					float num = std::atof( lexer->GetCurrentToken( ).ToString( ).c_str( ) );

					// Set value
					if ( numToken.Equals( "UIMax" ) )
					{
						prop.mTraits.UIMax = num;
					}

					if ( numToken.Equals( "UIMin" ) )
					{
						prop.mTraits.UIMin = num; 
					}
				}

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

	// Get property type from enum and handle special cases differently
	PropertyType type = Property::GetTypeFromString( prop.mType );

	switch ( type )
	{
		case PropertyType::AssetHandle:
		{
			// Get opening template token
			if ( !lexer->RequireToken( TokenType::Token_LessThan, true ) )
			{
				return;
			}

			// Append template token
			prop.mTypeAppend += lexer->GetCurrentToken( ).ToString( );

			// Get template type 
			if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
			{
				return;
			}

			// Store template type
			std::string mTemplateType = lexer->GetCurrentToken( ).ToString( );
			{
				Token curToken = lexer->GetCurrentToken( );
				Token nextToken = lexer->PeekAtNextToken( );
				while ( curToken.IsType( TokenType::Token_Identifier ) && nextToken.IsType( TokenType::Token_DoubleColon ) )
				{
					// Set to double color token
					curToken = lexer->GetNextToken( );

					// Store in templated type
					mTemplateType += curToken.ToString( );

					// Get next identifier token
					curToken = lexer->GetNextToken( );

					// Peek at next token
					nextToken = lexer->PeekAtNextToken( );
				}
			}

			// Get actual type
			if ( !lexer->RequireToken( TokenType::Token_Identifier ) )
			{
				return;
			} 

			// Should have full templated name now, so append
			prop.mTypeAppend += mTemplateType + lexer->GetCurrentToken( ).ToString( );

			// Grab closing token
			if ( !lexer->RequireToken( TokenType::Token_GreaterThan, true ) )
			{
				return;
			}

			// Append and close type
			prop.mTypeAppend += lexer->GetCurrentToken( ).ToString( ); 
		} break;
	}

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
	// New function to inflate
	Function func;

	// Grab next token and make sure is parentheses
	if ( !lexer->RequireToken( TokenType::Token_OpenParen, true ) )
	{
		return;
	}

	// Leave constructors for now
	if ( lexer->PeekAtNextToken( ).Equals( "Constructor" ) || lexer->PeekAtNextToken().Equals( "Destructor" ) )
	{
		return;
	}

	// Just continue onto close paren token
	if ( !lexer->ContinueTo( TokenType::Token_CloseParen ) )
	{
		return;
	}

	// Should be at ret type now, need take the entire token including any namespaces for return type
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	} 

	// Grab return value
	{
		Token curToken = lexer->GetCurrentToken( );
		Token nextToken = lexer->PeekAtNextToken( );
		std::string retType = curToken.ToString( );

		while ( curToken.IsType( TokenType::Token_Identifier ) && nextToken.IsType( TokenType::Token_DoubleColon ) )
		{
			curToken = lexer->GetNextToken( );
			curToken = lexer->GetNextToken( );
			nextToken = lexer->PeekAtNextToken( );

			// Append to retType
			retType += "::" + curToken.ToString( );
		}

		// Check for references and pointers
		curToken = lexer->GetNextToken( );
		while ( curToken.IsType( TokenType::Token_Asterisk ) ||
			curToken.IsType( TokenType::Token_Ampersand ) )
		{
			retType += curToken.ToString( );

			curToken = lexer->GetNextToken( );
		}

		// Add return type for function signature
		func.mSignature.mRetType = retType;
	}

	// Grab function name
	if ( !lexer->RequireToken( TokenType::Token_Identifier ) )
	{
		return;
	}

	// Add function name to signature
	func.mSignature.mFunctionName = lexer->GetCurrentToken( ).ToString( ); 

	// Need to parse parameter list now
	if ( !lexer->ContinueTo( TokenType::Token_OpenParen ) )
	{
		return;
	}

	// Grab parameter list
	{
		Token curToken = lexer->GetNextToken( );
		
		// Continue until we reach the end of the function signature ( close paren )
		while ( !curToken.IsType( TokenType::Token_CloseParen ) )
		{
			Token nextToken = lexer->PeekAtNextToken( );

			std::string paramType = curToken.ToString( );

			while ( curToken.IsType( TokenType::Token_Identifier ) && nextToken.IsType( TokenType::Token_DoubleColon ) )
			{
				curToken = lexer->GetNextToken( );
				curToken = lexer->GetNextToken( );
				nextToken = lexer->PeekAtNextToken( );

				// Append to retType
				paramType += "::" + curToken.ToString( );
			}

			// Check for references and pointers
			curToken = lexer->GetNextToken( );
			while ( curToken.IsType( TokenType::Token_Asterisk ) ||
				curToken.IsType( TokenType::Token_Ampersand ) )
			{
				paramType += curToken.ToString( );

				curToken = lexer->GetNextToken( );
			}

			// Add return type for function signature
			func.mSignature.mParameterList.push_back( paramType ); 

			// Now need to skip past param name as well as comma
			curToken = lexer->GetNextToken( );

			if ( curToken.IsType( TokenType::Token_Comma ) )
			{ 
				lexer->ContinueTo( TokenType::Token_Identifier );
			} 
		} 
	} 

	// All is well, so add the function to the class
	cls->AddFunction( func );
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

			// Copy over all functions into single map
			FunctionTable functions = c.second.mFunctions;
			if ( parentCls )
			{
				for ( auto& f : parentCls->mFunctions )
				{
					functions[ f.first ] = f.second;
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
				code += OutputTabbedLine( "Enjon::MetaProperty* props[] = " );
				code += OutputTabbedLine( "{" );

				for ( auto& prop : properties )
				{
					// Get property as string
					auto metaProp = Property::GetTypeFromString( prop.second.mType );
					std::string metaPropStr = Property::GetTypeAsString( metaProp ); 
					
					// Fill out property traits
					std::string traits = "MetaPropertyTraits( "; 
					traits += prop.second.mTraits.IsEditable ? "true" : "false";
					traits += ", ";
					traits += std::to_string(prop.second.mTraits.UIMin) + "f, ";
					traits += std::to_string(prop.second.mTraits.UIMax) + "f";
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
					code += OutputTabbedLine( "\tnew Enjon::MetaProperty( MetaPropertyType::" + metaPropStr + ", \"" + pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + " )" + endChar );
				} 

				// End property table
				code += OutputTabbedLine( "};\n" );
				code += OutputLine( "\n" );
	 
				// Assign properties
				code += OutputTabbedLine( "// Assign properties to class" ); 
				code += OutputTabbedLine( "cls->mProperties = Enjon::PropertyTable( props, props + cls->mPropertyCount );" ); 
			}

			// Iterate through all functions and output code
			if ( !functions.empty( ) )
			{
				code += OutputLine( "" );
				code += OutputTabbedLine( "// Construct functions" );
				code += OutputTabbedLine( "cls->mFunctionCount = " + std::to_string( functions.size( ) ) + ";" );
				
				// For each function
				for ( auto& func : functions )
				{
					// Get function name
					std::string fn = func.second.mSignature.mFunctionName;

					// Get return type
					std::string retType = func.second.mSignature.mRetType;

					// Build template signature
					std::string templateSignature = c.second.mName + ", " + retType ;

					// Add parameter list to template signature
					for ( auto& param : func.second.mSignature.mParameterList )
					{
						templateSignature += ", " + param;
					}

					code += OutputTabbedLine( "cls->mFunctions[ \"" + fn + "\" ] = new Enjon::MetaFunctionImpl< " + templateSignature + " >( &" + c.second.mName + "::" + fn + ", \"" + fn + "\" );" );
				}

			}

			// Formatting
			code += OutputLine( "" );

			// Set up typeid field
			code += OutputTabbedLine( "cls->mTypeId = Enjon::Object::GetTypeId< " + c.second.mName + " >( );\n" );

			// Return statement
			code += OutputLine( "\n\treturn cls;" );
			code += OutputLine( "}\n" ); 

			/*
				// Fill out object
				void Construct( MetaClass* cls, Object* object )
				{
					MetaClass* cls = const_cast< MetaClass* > ( object->Class( ) );
					PropertyTable& pt = cls->GetProperties( ); 

					// "Construct" object with default values
					for ( auto& prop : pt ) 
				}

				Has to return an object of a specific type...

			*/


			// Construct function
			code += OutputLine( "" );

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














