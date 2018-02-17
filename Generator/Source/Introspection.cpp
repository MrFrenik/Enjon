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
		
std::string Introspection::GetTypeAsString( PropertyType type )
{
	if ( mPropertyTypeStringMap.find( type ) != mPropertyTypeStringMap.end( ) )
	{
		return mPropertyTypeStringMap[ type ];
	} 

	return "Object";
}

//================================================================================================= 
		
PropertyType Introspection::GetTypeFromString( const std::string& str )
{
	// If base type found, then return
	if ( mPropertyTypeMap.find( str ) != mPropertyTypeMap.end( ) )
	{
		return mPropertyTypeMap[ str ];
	} 

	// If enum found, then is type enum
	if ( mEnums.find( str ) != mEnums.end( ) )
	{
		return PropertyType::Enum;
	} 

	// Default type object
	return PropertyType::Object;
}

//================================================================================================= 

#define STRING_TO_PROP( str, prop )\
mPropertyTypeMap[ str ] = PropertyType::prop;\
mPropertyTypeStringMap[ PropertyType::prop ] = #prop;

void Introspection::InitPropertyMap( )
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
	STRING_TO_PROP( "iVec3", iVec3 )
	STRING_TO_PROP( "Vec4", Vec4 )
	STRING_TO_PROP( "ColorRGBA32", ColorRGBA32 )
	STRING_TO_PROP( "UUID", UUID )
	STRING_TO_PROP( "String", String )
	STRING_TO_PROP( "Transform", Transform )
	STRING_TO_PROP( "Quaternion", Quat )
	STRING_TO_PROP( "AssetHandle", AssetHandle )
	STRING_TO_PROP( "EntityHandle", EntityHandle )
	STRING_TO_PROP( "Enum", Enum )
	STRING_TO_PROP( "Array", Array )
	STRING_TO_PROP( "HashMap", HashMap )
	STRING_TO_PROP( "Vector", Array )
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
		return mProperties[ name ];
	}

	return nullptr;
}

//=================================================================================================
		
void Class::AddProperty( Property* prop )
{
	if ( !HasProperty( prop->mName ) )
	{
		mProperties[ prop->mName ] = prop;
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
	InitPropertyMap( );
}

//=================================================================================================

void Introspection::SetTypeID( const u32& id )
{
	mLastObjectTypeId = id;
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

				// Parse enum
				else if ( token.Equals( "ENJON_ENUM" ) )
				{
					ParseEnum( lexer );
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
		mClasses[ className ] = Class( );
	}

	return &mClasses[ className ];
}

//=================================================================================================

void Introspection::ParseClassTraits( Lexer* lexer, ClassMarkupTraits* traits )
{
	// ContinueToken curToken = lexer->GetCurrentToken( );
	Token curToken = lexer->GetCurrentToken( );
	while ( !curToken.IsType( TokenType::Token_CloseParen ) && !curToken.IsType( TokenType::Token_EndOfStream ) )
	{ 
		// If identifier then process trait
		if ( curToken.IsType( TokenType::Token_Identifier ) )
		{
			// Namespace trait
			if ( curToken.Equals( "Namespace" ) )
			{
				// Require an equal sign
				if ( !lexer->RequireToken( TokenType::Token_Equal, true ) )
				{
					return;
				}

				// Require opening bracket for namespaces
				if ( !lexer->RequireToken( TokenType::Token_OpenBracket, true ) )
				{
					return;
				}

				// Read all namespace identifiers until close bracket is hit	
				Token token = lexer->GetNextToken( );
				while ( !token.IsType( TokenType::Token_CloseBracket ) )
				{
					// If identifier, then place into namespaces
					if ( token.IsType( TokenType::Token_Identifier ) )
					{
						traits->AddNamespaceQualifier( token.ToString( ) );
					}

					// Advance to next token
					token = lexer->GetNextToken( );
				}
			} 

			// Construct
			if ( curToken.Equals( "Construct" ) )
			{
				traits->Construct( true );
			}
		}

		// Get next token after processing this one
		curToken = lexer->GetNextToken( );
	}
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
	ClassMarkupTraits traits;
	ParseClassTraits( lexer, &traits );

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

	// Get the qualified name for the class
	std::string qualifiedClassName = "";
	for ( auto& ns : traits.mNamespaceQualifiers )
	{
		qualifiedClassName += ns + "::";
	}
	qualifiedClassName += classToken.ToString( );

	// Get new class created
	Class* cls = const_cast< Class* >( AddClass( qualifiedClassName ) );

	// Set contents path of class ( include directory )
	cls->mFilePath = lexer->GetContentsPath( );

	// Add markup traits
	cls->mTraits = traits;

	// Set name of token
	cls->mName = classToken.ToString( );

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
			bool consumed = false;
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

					consumed = true;
				}
			} 

			superToken = lexer->GetCurrentToken( );

			cls->mParent = parentString + ( consumed ? superToken.ToString( ) : "" );

			// If is an application, then need to record that
			if ( cls->mParent.compare( "Application" ) == 0 || cls->mParent.compare( "Enjon::Application" ) == 0 )
			{
				cls->mIsApplication = true;
			}
		}
	} 

	// Continue until open brace; if none hit, remove class
	if ( !lexer->ContinueTo( TokenType::Token_OpenBrace ) )
	{
		RemoveClass( qualifiedClassName );

		return;
	} 

	// Need to push scope now, since open brace has been found
	Class::PushScope( ); 

	// Continue to class body tag, if not found then remove class and return
	//if ( !lexer->ContinueToIdentifier( "ENJON_CLASS_BODY" ) )
	//{
	//	// Remove class
	//	RemoveClass( qualifiedClassName );

	//	return;
	//}

	std::vector<std::string> identifiers{ "ENJON_CLASS_BODY", "ENJON_COMPONENT", "ENJON_MODULE_BODY" };
	if ( !lexer->ContinueToAnyIdentifier( identifiers ) )
	{
		// Remove class
		RemoveClass( qualifiedClassName );

		return;
	}

	// Parse remainder of class body
	ParseClassBody( lexer, cls );

	// Class object identifier
	cls->mObjectTypeId = mLastObjectTypeId++;

	// Pop scope from class
	Class::PopScope( );
}

//=================================================================================================

const Class* Introspection::FindApplicationClass( )
{
	// Return first instance of application
	for ( auto& c : mClasses )
	{
		if ( c.second.mIsApplication )
		{
			return &c.second;
		}
	}

	return nullptr;
}

//=================================================================================================
		
void Introspection::ParseClassBody( Lexer* lexer, Class* cls )
{ 
	std::cout << "Class name: " << cls->mName << "\n";
	std::cout << "Lexer current token: " << lexer->GetCurrentToken( ).ToString( ) << "\n";

	// Look for specific other body modifiers as well
	if ( lexer->GetCurrentToken( ).Equals( "ENJON_COMPONENT" ) )
	{
		cls->mMetaClassType = MetaClassType::Component;
	}
	else if ( lexer->GetCurrentToken( ).Equals( "ENJON_MODULE_BODY" ) )
	{
		cls->mMetaClassType = MetaClassType::Application; 
		std::cout << "Was here\n";
	}
	else
	{
		cls->mMetaClassType = MetaClassType::Object;
	}

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
			case TokenType::Token_CloseBrace:
			{
				// Done parsing class
				if ( lexer->PeekAtNextToken( ).IsType( TokenType::Token_SemiColon ) )
				{
					isParsing = false;
				}
			};

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

PropertyType Introspection::GetPropertyType( Lexer* lexer )
{ 
	// Get property type from identifier token string
	std::string propType = lexer->GetCurrentToken().ToString( );

	// Need to check if is an array type
	if ( IsPropertyArrayType( lexer ) )
	{
		return PropertyType::Array;
	}
	// Need to check if is an map type
	else if ( IsPropertyHashMapType( lexer ) )
	{
		return PropertyType::HashMap;
	} 
	else
	{ 
		// Get property type 
		PropertyType type = GetTypeFromString( propType ); 

		return type;
	}
}

//====================================================================================================

bool Introspection::IsPropertyHashMapType( Lexer* lexer )
{ 
	return lexer->GetCurrentToken( ).Equals( "HashMap" ); 
}

//=================================================================================================

bool Introspection::IsPropertyArrayType( Lexer* lexer )
{
	// To be determined
	bool isArray = false; 

	// Dynamic array ( Vector ) is simple enough
	if ( lexer->GetCurrentToken( ).Equals( "Vector" ) )
	{
		return true;
	}

	// Check for static array / need to be able to peek ahead far enough to get to end of property declaration (';')
	Token cacheToken = lexer->GetCurrentToken( );
	Token tokenPeek = lexer->GetNextToken( );
	bool foundOpenBracket = false;
	bool foundCloseBracket = false;
	while ( tokenPeek.mType != TokenType::Token_SemiColon )
	{
		if ( tokenPeek.mType == TokenType::Token_OpenBracket )
		{
			foundOpenBracket = true;
		}
		if ( tokenPeek.mType == TokenType::Token_CloseBracket )
		{
			foundCloseBracket = true;
		}

		if ( foundOpenBracket && foundCloseBracket )
		{
			isArray = true;
			break;
		}

		// Get next token
		tokenPeek = lexer->GetNextToken( ); 
	} 

	// Reset to cached token
	lexer->SetToken( cacheToken );

	return isArray;
}

//=================================================================================================

void Introspection::ParseProperty( Lexer* lexer, Class* cls )
{ 
	// Property traits to fill out
	PropertyTraits traits;

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
					traits.IsEditable = true;
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
						traits.UIMax = num;
					}

					if ( numToken.Equals( "UIMin" ) )
					{
						traits.UIMin = num; 
					}
				} 
			}
		}
	} 

	// Get rid of const qualifier here
	if ( lexer->PeekAtNextToken( ).Equals( "const" ) )
	{
		lexer->GetNextToken( );
	}

	// Need to strip away all namespaces from property
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	} 

	// Consume all namespace qualifiers
	lexer->ConsumeAllNamespaceQualifiers( );

	// Get property type of current token
	if ( !lexer->RequireToken( TokenType::Token_Identifier ) )
	{
		return; 
	}
 
	// Grab current token to cache type 
	Token curToken = lexer->GetCurrentToken( );

	// Get property type from lexer
	PropertyType type = GetPropertyType( lexer ); 

	Property* prop = nullptr; 

	if ( type == PropertyType::Array )
	{
		prop = new ArrayProperty( );
	}
	else if ( type == PropertyType::HashMap )
	{
		prop = new HashMapProperty( );
	}
	else
	{
		prop = new Property( );
	}

	// Set property traits
	prop->mTraits = traits;

	// Get string of property type to store
	prop->mTypeRaw = curToken.ToString( );

	// Set property type 
	prop->mType = type; 

	// Specific parsing of non-primitive types  
	switch ( type )
	{
		default:
		{
			// Consume type
			Token consumeToken = lexer->GetNextToken( );
		} break;
		case PropertyType::AssetHandle:
		{
			Token consumeToken = lexer->GetNextToken( );

			// Get opening template token
			if ( !lexer->RequireToken( TokenType::Token_LessThan, true ) )
			{
				delete prop;
				return;
			}

			// Append template token
			prop->mTypeAppend += lexer->GetCurrentToken( ).ToString( );

			// Get template type 
			if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
			{
				delete prop;
				return;
			}

			// Store template type
			bool consumedQualifiers = false;
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

					consumedQualifiers = true;
				}
			}

			// Get actual type
			if ( !lexer->RequireToken( TokenType::Token_Identifier ) )
			{
				delete prop;
				return;
			} 

			if ( consumedQualifiers )
			{
				// Should have full templated name now, so append
				prop->mTypeAppend += mTemplateType + lexer->GetCurrentToken( ).ToString( ); 
			}
			else
			{
				prop->mTypeAppend += mTemplateType;
			} 

			// Grab closing token
			if ( !lexer->RequireToken( TokenType::Token_GreaterThan, true ) )
			{
				delete prop;
				return;
			}

			// Append and close type
			prop->mTypeAppend += lexer->GetCurrentToken( ).ToString( ); 
		} break;

		case PropertyType::Array:
		{ 
			// Will be the actual property type
			Token token = lexer->GetCurrentToken( ); 

			// Cast to array property
			ArrayProperty* arrProp = static_cast<ArrayProperty*> ( prop );

			// Dynamic array property
			if ( token.Equals( "Vector" ) )
			{
				arrProp->mArraySizeType = ArraySizeType::Dynamic; 

				// Need to the templated argument begin
				if ( !lexer->RequireToken( TokenType::Token_LessThan, true ) )
				{
					delete arrProp;
					return;
				} 


				// This part is the problem... This needs to be recursive and stop until the LAST '>' is found of this templated type.
				// Get the property string up to the closing bracket
				std::string propertyString = "";
				token = lexer->GetNextToken();

				// Consume namespace qualifiers
				if ( token.Equals( "Enjon" ) )
				{
					lexer->ConsumeAllNamespaceQualifiers( );

					// Get next token after initial namespace qualifier
					token = lexer->GetCurrentToken( );
				} 

				// If an AssetHandle
				bool isAssetHandle = false;
				if ( token.Equals( "AssetHandle" ) )
				{
					isAssetHandle = true;
				} 

				while ( token.mType != TokenType::Token_GreaterThan )
				{
					propertyString += token.ToString( );
					token = lexer->GetNextToken( );
				}

				// Now that the first greater than has been found, must continue until the LAST '>' is found
				while ( lexer->PeekAtNextToken( ).IsType( TokenType::Token_GreaterThan ) || lexer->PeekAtNextToken().IsType( TokenType::Token_Asterisk ) )
				{
					token = lexer->GetNextToken( );
					propertyString += token.ToString( );
				}

				// Set property type of array
				arrProp->mPropertyTypeRaw = propertyString; 

				// Need to store the property type as well
				auto splitVector = SplitString( propertyString, ":" );
				// The last element is the actual unqualified type
				PropertyType pt = GetTypeFromString( splitVector.back( ) );

				// Set property type of array
				arrProp->mArrayPropertyType = pt; 

				if ( isAssetHandle )
				{
					arrProp->mArrayPropertyType = PropertyType::AssetHandle;
				}
			}
			// Fixed array property
			else
			{
				// Fixed size array type
				arrProp->mArraySizeType = ArraySizeType::Fixed; 

				// Need to grab type of array
				std::string propertyString = "";
				token = lexer->GetCurrentToken( );

				// Getting the type of the property
				// Hate this, but will generalize later to recursive property define calls...
				bool consumeTypeToken = true;
				if ( token.Equals( "AssetHandle" ) )
				{ 
					// NOTE(): Gross...
					consumeTypeToken = false;

					// Beginning of property string for this array
					std::string propertyString = "Enjon::AssetHandle<";

					// Need to consume the property type now
					lexer->GetNextToken( ); 

					// Need to the templated argument begin
					if ( !lexer->RequireToken( TokenType::Token_LessThan, true ) )
					{
						delete arrProp;
						return;
					}

					// Get the property string up to the closing bracket
					token = lexer->GetNextToken( );
					while ( token.mType != TokenType::Token_GreaterThan )
					{
						propertyString += token.ToString( );
						token = lexer->GetNextToken( );
					}
					propertyString += ">";

					// Set property type of array
					arrProp->mPropertyTypeRaw = propertyString; 

					// Need to store the property type as well
					auto splitVector = SplitString( propertyString, "::" );
					// The last element is the actual unqualified type
					PropertyType pt = GetTypeFromString( splitVector.back( ) );

					// Set property type of array
					arrProp->mArrayPropertyType = PropertyType::AssetHandle; 
				}
				else
				{
					arrProp->mPropertyTypeRaw = arrProp->mTypeRaw;
					arrProp->mArrayPropertyType = GetTypeFromString( SplitString( arrProp->mPropertyTypeRaw, "::" ).back( ) );
				}

				// Consume type token
				if ( consumeTypeToken )
				{
					lexer->GetNextToken( );
				}

				// Now need to cache off identifier token for later
				if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
				{
					delete arrProp;
					return;
				} 
				Token nameToken = lexer->GetCurrentToken( );

				// Now need to search for size string
				if ( !lexer->RequireToken( TokenType::Token_OpenBracket, true ) )
				{
					delete arrProp;
					return;
				}

				std::string sizeString = "";
				Token ct = lexer->GetNextToken( );
				while ( ct.mType != TokenType::Token_CloseBracket )
				{
					sizeString += ct.ToString( );
					ct = lexer->GetNextToken( );
				}

				// Set size string of array property
				arrProp->mSizeString = sizeString;

				// Restore name token
				lexer->SetToken( nameToken ); 
			} 
		} break;

		case PropertyType::HashMap:
		{
			// Will be the actual property type
			Token token = lexer->GetNextToken( );

			// Need to have a better recursive way of handling filling out the property information for this...  
			HashMapProperty* mapProp = static_cast<HashMapProperty*> ( prop );

			if ( token.Equals( "HashMap" ) )
			{ 
				// Need to the templated argument begin
				if ( !lexer->RequireToken( TokenType::Token_LessThan, true ) )
				{
					delete mapProp;
					return;
				}

				// Get the all tokens up to the closing bracket
				std::string keyValueString = "";
				token = lexer->GetNextToken( );
				while ( token.mType != TokenType::Token_GreaterThan )
				{
					keyValueString += token.ToString( );
					token = lexer->GetNextToken( );
				}

				// Split on comma for key and value
				auto splitVector = SplitString( keyValueString, "," );

				// If not a key and value, then delete and return
				if ( splitVector.size( ) != 2 )
				{
					delete mapProp;
					return;
				}

				// Get key and value strings separately
				std::string keyString = splitVector.at( 0 );
				std::string valueString = splitVector.at( 1 );

				// Set raw types of map
				mapProp->mKeyPropertyTypeRaw = keyString; 
				mapProp->mValuePropertyTypeRaw = valueString; 

				// Need to store the property type as well
				auto keySplit = SplitString( keyString, "::" );
				auto valueSplit = SplitString( valueString, "::" );

				// The last element is the actual unqualified type
				PropertyType kpt = GetTypeFromString( keySplit.back( ) );
				PropertyType vpt = GetTypeFromString( valueSplit.back( ) );

				// Set property type of array
				mapProp->mKeyPropertyType = kpt;
				mapProp->mValuePropertyType = vpt;
			}
			else
			{
				delete mapProp;
				return;
			}

		} break;
	}

	// TODO(): Pointer types / Const references
	if ( lexer->PeekAtNextToken( ).IsType( TokenType::Token_Asterisk ) || lexer->PeekAtNextToken( ).IsType( TokenType::Token_Ampersand ) )
	{
		// Store pointer
		if ( lexer->PeekAtNextToken( ).IsType( TokenType::Token_Asterisk ) )
		{
			prop->mTraits.IsPointer = true;
		}

		lexer->GetNextToken( );
	}
		
	// Get property name
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Grab token for property name
	curToken = lexer->GetCurrentToken( ); 

	// Get name
	prop->mName = curToken.ToString( ); 

	std::cout << prop->mName << ": " << GetTypeAsString( prop->mType ) << "\n";

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
 
			// Consume the const
			bool hadConst = false;
			if ( curToken.Equals( "const" ) )
			{
				paramType += " ";
				curToken = lexer->GetNextToken( );
				hadConst = true;
			}

			while ( curToken.IsType( TokenType::Token_Identifier ) && nextToken.IsType( TokenType::Token_DoubleColon ) )
			{
				curToken = lexer->GetNextToken( );
				curToken = lexer->GetNextToken( );
				nextToken = lexer->PeekAtNextToken( );

				// Append to retType
				paramType += "::" + curToken.ToString( ); 
			} 

			if ( hadConst )
			{
				paramType += curToken.ToString( ); 
			} 

			// Check for references and pointers
			curToken = lexer->GetNextToken( );

			if ( curToken.IsType( TokenType::Token_Ampersand ) || curToken.IsType( TokenType::Token_Asterisk ) )
			{
				paramType += curToken.ToString( );
				curToken = lexer->GetNextToken( );
			}

			while ( curToken.IsType( TokenType::Token_Asterisk ) ||
				curToken.IsType( TokenType::Token_Ampersand ) )
			{
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

void Introspection::ParseEnum( Lexer* lexer )
{ 
	// Value of previously visited element in enum
	u32 previousEnumElementValue = 0;

	// Grab next token and make sure is parentheses
	if ( !lexer->RequireToken( TokenType::Token_OpenParen, true ) )
	{
		return;
	}

	// Continue on to closed paren for now
	if ( !lexer->ContinueTo( TokenType::Token_CloseParen ) )
	{ 
		return;
	} 

	// Need to consume enum identifier declaration 
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// If class, then consume
	if ( lexer->PeekAtNextToken( ).Equals( "class" ) )
	{
		lexer->GetNextToken( );
	}

	// Should be on enum name now, so make sure we're at an identifier
	if ( !lexer->RequireToken( TokenType::Token_Identifier, true ) )
	{
		return;
	}

	// Get current token
	Token curToken = lexer->GetCurrentToken( );

	// Grab enum name from token
	std::string enumName = curToken.ToString( );

	// Add enum
	Enum* enm = const_cast< Enum*> ( AddEnum( enumName ) );
	enm->mName = enumName;

	// Continue to open brace
	if ( !lexer->ContinueTo( TokenType::Token_OpenBrace ) )
	{
		return;
	}

	// Now at elements for enum
	curToken = lexer->GetCurrentToken( );
	while ( curToken.mType != TokenType::Token_SemiColon )
	{ 
		// Element name
		if ( curToken.mType == TokenType::Token_Identifier )
		{ 
			// Element to inflate
			EnumElement element;
			
			// Set its name
			element.mElementName = curToken.ToString( );

			// Try and see if the value is set; if so, then need to set our previous value to this value
			if ( lexer->PeekAtNextToken().mType == TokenType::Token_Equal )
			{
				// Consume Equal sign
				lexer->GetNextToken( );

				// Store value of next token
				element.mValue = atoi( lexer->GetCurrentToken( ).ToString( ).c_str( ) );

				// Increment previous value by 1
				previousEnumElementValue++;
			}
			// Otherwise, need to set it to the previousEnumElement value and then increment it
			else
			{
				element.mValue = previousEnumElementValue++;
			} 

			// Push element into enum declaration
			enm->mElements.push_back( element );
		} 

		// Get next token
		curToken = lexer->GetNextToken( );
	} 
}

//=================================================================================================

bool Introspection::EnumExists( const std::string& enumName )
{
	return ( mEnums.find( enumName ) != mEnums.end( ) );
}

//=================================================================================================

void Introspection::RemoveEnum( const std::string& enumName )
{
	mEnums.erase( enumName ); 
}

//=================================================================================================

const Enum* Introspection::AddEnum( const std::string& enumName )
{
	if ( !EnumExists( enumName ) )
	{
		mEnums[ enumName ] = Enum( );
	}

	return &mEnums[ enumName ]; 
}

//================================================================================================= 

const Enum* Introspection::GetEnum( const std::string& name )
{
	if ( EnumExists( name ) )
	{
		return &mEnums[ name ];
	}

	return nullptr; 
}

//=================================================================================================

void Introspection::Compile( const ReflectionConfig& config )
{
	///////////////////////////////////////////
	// ENUMS //////////////////////////////////

	{
		// Grab output path
		std::string enumOutputPath = config.mOutputDirectory + "/" + config.mProjectName + "_Enum_generated.gen";

		// Open file
		std::ofstream f( enumOutputPath );
		if ( f )
		{
			// Build code
			std::string code;

			// Output generation file for enums
			for ( auto& e : mEnums )
			{
				std::string qualifiedName = e.second.mName;
				std::string structName = "Enum_" + qualifiedName + "_Structure";

				// Constructor
				code += OutputLine( "struct " + structName );
				code += OutputLine( "{" );
				code += OutputTabbedLine( structName + "()" );
				code += OutputTabbedLine( "{" ); 
				for ( auto& elem : e.second.mElements )
				{
					code += OutputTabbedLine( "\tmElements.push_back( MetaPropertyEnumElement( \"" + elem.mElementName + "\", " + std::to_string(elem.mValue) + " ) );" );
				}
				code += OutputTabbedLine( "}" );

				// Get elements
				code += OutputTabbedLine( "const Vector< MetaPropertyEnumElement >& Elements() const" );
				code += OutputTabbedLine( "{" ); 
				code += OutputTabbedLine( "\t return mElements;" );
				code += OutputTabbedLine( "}" ); 

				code += OutputTabbedLine( "" );
				code += OutputTabbedLine( "Vector< MetaPropertyEnumElement > mElements;" ); 

				code += OutputLine( "};" );
				code += OutputLine( "" );
			} 

			// Write final to file
			f.write( code.c_str( ), code.length( ) );
		} 
	} 

	// Classes
	for ( auto& c : mClasses )
	{
		// Get qualified name of class
		std::string qualifiedName = c.second.GetQualifiedName( ); 
 
		// Grab output path
		std::string outputPath = config.mOutputDirectory + "/" + FindReplaceAll( qualifiedName, "::", "_" ) + "_generated.gen";

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
			code += OutputLine( "// " + qualifiedName );
			code += OutputLine( "template <>" );
			code += OutputLine( "MetaClass* Object::ConstructMetaClass< " + qualifiedName + " >( )" );
			code += OutputLine( "{" );
			code += OutputTabbedLine( "MetaClass* cls = new MetaClass( );\n" ); 

			// Construct properties
			code += OutputTabbedLine( "// Construct properties" );
			code += OutputTabbedLine( "cls->mPropertyCount = " + std::to_string( properties.size( ) ) + ";" ); 

			// Construct function
			if ( c.second.mTraits.mConstruct )
			{
				code += OutputLine( "" );
				code += OutputTabbedLine( "cls->mConstructor = ([](){" );
				code += OutputTabbedLine( "\treturn new " + qualifiedName + "();" );
				code += OutputTabbedLine( "});" );
				code += OutputLine( "" ); 
			}

			// Iterate through properties and output code
			u32 index = 0;
			if ( !properties.empty( ) )
			{
				// Properties
				code += OutputTabbedLine( "cls->mProperties.resize( cls->mPropertyCount );" ); 
				for ( auto& prop : properties )
				{
					// Get property as string
					auto metaProp = prop.second->mType;
					std::string metaPropStr = GetTypeAsString( metaProp ); 

					// Check for enum here
					const Enum* enm = GetEnum( prop.second->mTypeRaw );
					if ( enm )
					{
						metaProp = PropertyType::Enum;
						metaPropStr = GetTypeAsString( metaProp );
					}
					
					// Fill out property traits
					std::string traits = "MetaPropertyTraits( "; 
					traits += prop.second->mTraits.IsEditable ? "true" : "false";
					traits += ", ";
					traits += std::to_string(prop.second->mTraits.UIMin) + "f";
					traits += ", ";
					traits += std::to_string(prop.second->mTraits.UIMax) + "f";
					traits += ", ";
					traits += prop.second->mTraits.IsPointer ? "true" : "false";
					traits += " )"; 

					// Get property name
					std::string pn = prop.second->mName;

					// Get class name
					std::string cn = qualifiedName;

					// Get property index
					std::string pi = std::to_string( index++ );

					// Get end character
					std::string endChar = index <= properties.size( ) - 1 ? "," : "";

					// Output line based on meta property type
					switch ( metaProp )
					{
						case PropertyType::AssetHandle:
						{
							code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaPropertyAssetHandle" + prop.second->mTypeAppend + "( MetaPropertyType::" + metaPropStr + ", \"" 
															+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + " );" ); 
						} break;

						case PropertyType::Enum:
						{
							// Need to get the enum to which this property belongs based on its type
							const Enum* enm = GetEnum( prop.second->mTypeRaw );
							if ( enm )
							{
								code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaPropertyEnum( MetaPropertyType::" + metaPropStr + ", \"" 
																+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + ", Enum_" + enm->mName + "_Structure().Elements(), \"" + prop.second->mTypeRaw + "\" );" ); 
							}
							
						} break;

						case PropertyType::HashMap:
						{ 
							HashMapProperty* mp = static_cast< HashMapProperty* > ( prop.second ); 

							std::string keyPropStr = GetTypeAsString( mp->mKeyPropertyType );
							std::string valPropStr = GetTypeAsString( mp->mValuePropertyType );
							
							// Check for enum here
							const Enum* keyEnm = GetEnum( mp->mKeyPropertyTypeRaw );
							if ( keyEnm )
							{
								keyPropStr = "Enum";
							}

							const Enum* valEnm = GetEnum( mp->mValuePropertyTypeRaw );
							if ( valEnm )
							{
								valPropStr = "Enum";
							}

							std::string keyProxyString = "";
							std::string valProxyString = ""; 

							if ( keyEnm )
							{
								keyProxyString += "new Enjon::MetaPropertyEnum( MetaPropertyType::" + keyPropStr + ", " 
															+ "\"KeyProxy\", 0, 0, MetaPropertyTraits(false, 0, 0), " + "Enum_" + keyEnm->mName + "_Structure().Elements(), \"" + mp->mKeyPropertyTypeRaw + "\" )"; 
							}
							else
							{
								keyProxyString += "new Enjon::MetaProperty( MetaPropertyType::" + keyPropStr + ", \"KeyProxy\", 0, 0, MetaPropertyTraits(false, 0, 0) )"; 
							}

							if ( valEnm )
							{
								valProxyString += "new Enjon::MetaPropertyEnum( MetaPropertyType::" + valProxyString + ", " 
															+ "\"ValueProxy\", 0, 0, MetaPropertyTraits(false, 0, 0), " + "Enum_" + valEnm->mName + "_Structure().Elements(), \"" + mp->mValuePropertyTypeRaw + "\" )"; 
							}
							else
							{
								valProxyString += "new Enjon::MetaProperty( MetaPropertyType::" + valPropStr + ", \"ValueProxy\", 0, 0, MetaPropertyTraits(false, 0, 0) )"; 
							}

							code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaPropertyHashMap< " + mp->mKeyPropertyTypeRaw + ", " + mp->mValuePropertyTypeRaw + " >( MetaPropertyType::" + metaPropStr + ", \"" 
															+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + ", MetaPropertyType::" + keyPropStr + ", MetaPropertyType::" + valPropStr + ", " + keyProxyString + ", "   
															+ valProxyString + " );" ); 

						} break;

						case PropertyType::Array:
						{ 
							ArrayProperty* ap = static_cast<ArrayProperty*>( prop.second );
							std::string arraySizeType = ap->mArraySizeType == ArraySizeType::Dynamic ? "ArraySizeType::Dynamic" : "ArraySizeType::Fixed";
							std::string arrayPropStr = GetTypeAsString( ap->mArrayPropertyType );

							std::string propertyProxyString = "";

							switch ( ap->mArrayPropertyType )
							{
								default:
								{
									propertyProxyString += "new Enjon::MetaProperty( MetaPropertyType::" + arrayPropStr + ", \"Proxy\", 0, 0, MetaPropertyTraits(false, 0, 0) )"; 
								} break;

								case PropertyType::AssetHandle:
								{ 
									// This is gon' be ugly...
									// Original string = "AssetHandle< someType >
									auto split = SplitString( ap->mPropertyTypeRaw, "<" ).at(1);
									split = SplitString( split, ">" ).at( 0 );
									propertyProxyString += "new Enjon::MetaPropertyAssetHandle< " + split + " >( MetaPropertyType::" + arrayPropStr + ", \"Proxy\", 0, 0, MetaPropertyTraits(false, 0, 0) )"; 
								} break;
							}

							switch ( ap->mArraySizeType )
							{
								case ArraySizeType::Dynamic:
								{
									code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaPropertyArray< " + ap->mPropertyTypeRaw + " >( MetaPropertyType::" + metaPropStr + ", \"" 
																	+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + ", " + arraySizeType + ", MetaPropertyType::" 
																	+ arrayPropStr + ", " + propertyProxyString + " );" ); 

								} break;

								case ArraySizeType::Fixed:
								{
									code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaPropertyArray< " + ap->mPropertyTypeRaw + " >( MetaPropertyType::" + metaPropStr + ", \"" 
																	+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + ", " + arraySizeType + ", MetaPropertyType::" 
																	+ arrayPropStr + ", " + propertyProxyString + ", usize( " + ap->mSizeString + " ) );" ); 
								} break;
							}
						} break;

						default:
						{

							if ( prop.second->mTraits.IsPointer )
							{
								code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaPropertyPointer< " + cn + ", " + prop.second->mTypeRaw + " >( MetaPropertyType::" + metaPropStr + ", \"" 
																+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + ", &" + cn + "::" + pn + " );" ); 
							}
							else
							{
								code += OutputTabbedLine( "cls->mProperties[ " + pi + " ] = new Enjon::MetaProperty( MetaPropertyType::" + metaPropStr + ", \"" 
																+ pn + "\", ( u32 )&( ( " + cn + "* )0 )->" + pn + ", " + pi + ", " + traits + " );" ); 
							}
							//cls->mProperties[ 7 ] = new Enjon::MetaPropertyPointer<RigidBody, CollisionShape>( MetaPropertyType::Object, "mShape", ( u32 )&( ( RigidBody* )0 )->mShape, 7, MetaPropertyTraits( false, 0.000000f, 0.000000f, true ), &RigidBody::mShape );
						} break;
					}
				} 

				// Formatting
				code += OutputLine( "" ); 
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
					std::string templateSignature = qualifiedName + ", " + retType ;

					// Add parameter list to template signature
					for ( auto& param : func.second.mSignature.mParameterList )
					{
						templateSignature += ", " + param;
					}

					code += OutputTabbedLine( "cls->mFunctions[ \"" + fn + "\" ] = new Enjon::MetaFunctionImpl< " + templateSignature + " >( &" + qualifiedName + "::" + fn + ", \"" + fn + "\" );" );
				}

			}

			// Formatting
			code += OutputLine( "" );

			// Set up typeid field
			code += OutputTabbedLine( "cls->mTypeId = " + std::to_string( c.second.mObjectTypeId ) + ";\n" );

			// Set up meta class type field
			switch ( c.second.mMetaClassType )
			{
				case MetaClassType::Object:
				{
					code += OutputTabbedLine( "cls->mMetaClassType = Enjon::MetaClassType::Object;\n" ); 
				} break; 
				case MetaClassType::Component:
				{
					code += OutputTabbedLine( "cls->mMetaClassType = Enjon::MetaClassType::Component;\n" ); 
				} break; 
				case MetaClassType::Application:
				{
					code += OutputTabbedLine( "cls->mMetaClassType = Enjon::MetaClassType::Application;\n" ); 
				} break; 
			} 
 
			// Set up name field
			code += OutputTabbedLine( "cls->mName = \"" + qualifiedName + "\";\n" );

			// Return statement
			code += OutputTabbedLine( "return cls;" );
			code += OutputLine( "}\n" ); 

			// GetClassInternal()
			code += OutputLine( "// GetClassInternal" );
			code += OutputLine( "const MetaClass* " + qualifiedName + "::GetClassInternal() const" );
			code += OutputLine( "{" );
			code += OutputTabbedLine( "MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance()->GetMetaClassRegistry() );" );
			code += OutputTabbedLine( "const MetaClass* cls = mr->Get< " + qualifiedName + " >( );" );
			code += OutputTabbedLine( "if ( !cls )" );
			code += OutputTabbedLine( "{" );
			code += OutputTabbedLine( "\tcls = mr->RegisterMetaClass< " + qualifiedName + " >( );" );
			code += OutputTabbedLine( "}" );
			code += OutputTabbedLine( "return cls;" );
			code += OutputLine( "}" );

			// GetTypeId()
			code += OutputLine( "" );
			code += OutputLine( "// MetaClassRegistry::GetTypeId" );
			code += OutputLine( "template <>" );
			code += OutputLine( "Enjon::u32 MetaClassRegistry::GetTypeId< " + qualifiedName + " >() const" );
			code += OutputLine( "{" );
			code += OutputTabbedLine( "return " + std::to_string(c.second.mObjectTypeId) + ";" );
			code += OutputLine( "}" ); 

			// Write to file
			f.write( code.c_str( ), code.length( ) );
		} 
	} 
}
		
std::string Introspection::OutputLinkedHeader( const ReflectionConfig& config )
{
	std::string code = "";

	code += OutputLine( "// @file " + config.mProjectName + "_Generated.cpp" );
	code += OutputLine( "// Copyright 2016-2017 John Jackson. All Rights Reserved." );
	code += OutputLine( "// This file has been generated. All modifications will be lost." );
	code += OutputLine( "" );

	return code;
}

//=================================================================================================

void Introspection::Link( const ReflectionConfig& config )
{
	// Open link file to write to
	std::string enginePath = config.mEnginePath + "/Include/Engine.h";
	std::string typesPath = config.mEnginePath + "/Include/System/Types.h";
	std::string definesPath = config.mEnginePath + "/Include/Defines.h";
	std::string linkFilePath = config.mLinkedDirectory + "/" + config.mProjectName + "_Generated.cpp"; 
	std::string enumDefinesPath = config.mOutputDirectory + "/" + config.mProjectName + "_Enum_generated.gen";
	std::ofstream f( linkFilePath ); 

	// Code to write to file
	std::string code = "";

	// Write header for Linked file
	code += OutputLinkedHeader( config ); 

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

	// Write enum structure definitions file
	{
		// Grab enum contents
		std::string enumContents = ReadFileIntoString( enumDefinesPath.c_str( ) );
		// Append
		code += OutputLine( enumContents );
	}

	// Iterate classes and grab compiled intermediate files
	for ( auto& c : mClasses )
	{
		// Get qualified name of class
		std::string qualifiedName = c.second.GetQualifiedName( );

		// Grab output path
		std::string inputPath = config.mOutputDirectory + "/" + FindReplaceAll( qualifiedName, "::", "_" ) + "_generated.gen";

		// Open file
		std::string fileContents = ReadFileIntoString( inputPath.c_str( ) ); 

		// Append contents
		code += OutputLine( fileContents );
	} 

	// Get application if exists
	const Class* applicationClass = FindApplicationClass( );

	// Only write static meta class binding call for enjon includes
	if ( config.mProjectName.compare( "Enjon" ) == 0 )
	{
		// Output Binding Function
		code += OutputLine( "// Binding function for Enjon::Object that is called at startup for reflection" );
		code += OutputLine( "void Object::BindMetaClasses()" );
		code += OutputLine( "{" );
		for ( auto& c : mClasses )
		{
			std::string qualifiedName = c.second.GetQualifiedName( );
			code += OutputTabbedLine( "Object::RegisterMetaClass< " + qualifiedName + " >();" ); 
		}
		code += OutputLine( "}" ); 
	} 

	// If application class exists, write binding/unbinding functions
	if ( applicationClass )
	{
		// Output Binding function
		code += OutputLine( "// Binding function for Application that is called at startup for reflection" );
		code += OutputLine( "Enjon::Result " + applicationClass->mName + "::BindApplicationMetaClasses()" );
		code += OutputLine( "{" );
		for ( auto& c : mClasses )
		{
			std::string qualifiedName = c.second.GetQualifiedName( );
			code += OutputTabbedLine( "Enjon::Object::BindMetaClass< " + qualifiedName + " >();" ); 
		}
		code += OutputTabbedLine( "return Enjon::Result::SUCCESS;" );
		code += OutputLine( "}" ); 

		// Formatting
		code += OutputLine( "" );

		// Output Unbinding function
		code += OutputLine( "// Unbinding function for Application that is called when reloading for reflection" );
		code += OutputLine( "Enjon::Result " + applicationClass->mName + "::UnbindApplicationMetaClasses()" );
		code += OutputLine( "{" );
		for ( auto& c : mClasses )
		{
			std::string qualifiedName = c.second.GetQualifiedName( );
			code += OutputTabbedLine( "Enjon::Object::UnbindMetaClass< " + qualifiedName + " >();" ); 
		}
		code += OutputTabbedLine( "return Enjon::Result::SUCCESS;" );
		code += OutputLine( "}" ); 
	}

	// Output linked code
	if ( f )
	{
		f.write( code.c_str( ), code.length( ) );
	}
}














