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

struct catalog
{
	char** Data;
	u32 MaxAllocatedSize;
	u32 CurrentSize;
};

struct catalog TypeCatalog; 

std::vector<std::string> output;

#define DEBUG_TOKEN_PRINT(Token) \
	printf("%.*s\n", Token.TextLength, Token.Text);

#define INDENT(x) for (int i = 0; i < x; i++) printf("\t"); 

enum token_type
{
	Token_Unknown,
	Token_Identifier,	
	Token_OpenParen,	
	Token_CloseParen,	
	Token_String,	
	Token_Colon,	
	Token_SemiColon,	
	Token_Asterisk,	
	Token_OpenBracket,	
	Token_CloseBracket,	
	Token_OpenBrace,	
	Token_CloseBrace,	
	Token_EndOfStream,	
};

struct token
{
	token_type Type;
	size_t TextLength;
	char *Text;	
};

struct tokenizer
{
	char *At;
}; 

inline void AddToCatalog(char* Type, catalog* Catalog)
{
	// Make sure that the size is within allocated bounds
	if (Catalog->CurrentSize == Catalog->MaxAllocatedSize)
	{
		Catalog->MaxAllocatedSize *= 2;
		Catalog->Data = (char**)realloc(Catalog->Data, sizeof(char*) * Catalog->MaxAllocatedSize);
	}	

	Catalog->Data[Catalog->CurrentSize++] = Type;
}

inline void RegisterTypeWithCatalog(token TypeToRegister, catalog* Catalog)
{
	// Make sure that is actually an identifier 
	if (TypeToRegister.Type != Token_Identifier) return;

	// Make the string to be registered
	char* RegisteredType = (char*)malloc(TypeToRegister.TextLength + 1);
	RegisteredType[TypeToRegister.TextLength] = '\0';
	strncpy(RegisteredType, TypeToRegister.Text, TypeToRegister.TextLength);

	// Search to make sure that type doesn't already exist in catalog
	for (u32 i = 0; i < Catalog->CurrentSize; i++)
	{
		if (StringEqual(RegisteredType, Catalog->Data[i], TypeToRegister.TextLength))
		{
			return;
		}
	}

	// Otherwise not found, need to add to catalog
	AddToCatalog(RegisteredType, Catalog);
} 

static void EatAllWhiteSpace(tokenizer *Tokenizer)
{
	for(;;)
	{
		if (IsWhiteSpace(Tokenizer->At[0]))
		{
			++Tokenizer->At;
		}
		else if ((Tokenizer->At[0] == '/') &&
				 (Tokenizer->At[1]) && 
				 (Tokenizer->At[1] == '/'))
		{
			Tokenizer->At += 2;
			while(Tokenizer->At[0] && 
				  !IsEndOfLine(Tokenizer->At[0]))
			{
				++Tokenizer->At;
			}
		}
		else if ((Tokenizer->At[0] == '/') && 
			     (Tokenizer->At[1]) && 
			     (Tokenizer->At[1] == '*'))
		{
			Tokenizer->At += 2;
			while(Tokenizer->At[0] && Tokenizer->At[1] &&
				  !(Tokenizer->At[0] == '*' && Tokenizer->At[1] == '/'))
			{
				++Tokenizer->At;
			}
			if (Tokenizer->At[0] == '*')
			{
				++Tokenizer->At;
			}
		}
		else
		{
			break;
		}
	}
}

static token GetToken(tokenizer *Tokenizer)
{
	EatAllWhiteSpace(Tokenizer);	

	token Token = {};
	Token.TextLength = 1;
	Token.Text = Tokenizer->At;
	char C = Tokenizer->At[0];
	++Tokenizer->At;

	switch(C)
	{
		case '(': 	{Token.Type = Token_OpenParen; }	break;
		case ')': 	{Token.Type = Token_CloseParen; } 	break;
		case ':': 	{Token.Type = Token_Colon; } 		break;
		case ';': 	{Token.Type = Token_SemiColon; }	break;
		case '*': 	{Token.Type = Token_Asterisk; }		break;
		case '{': 	{Token.Type = Token_OpenBrace; }	break;
		case '}': 	{Token.Type = Token_CloseBrace; }	break;
		case '[': 	{Token.Type = Token_OpenBracket; }	break;
		case ']': 	{Token.Type = Token_CloseBracket; }	break;
		case '\0': 	{Token.Type = Token_EndOfStream; } 	break;

		case '/': 
		{

		} break;

		case '"':
		{
			Token.Text = Tokenizer->At;

			while(Tokenizer->At[0] && 
				  Tokenizer->At[0] != '"')
			{
				if (Tokenizer->At[0] == '\\' &&
					Tokenizer->At[1])
				{
					++Tokenizer->At;
				}
				++Tokenizer->At;
			}

			Token.TextLength = Tokenizer->At - Token.Text;
			Token.Type = Token_String;

			if (Tokenizer->At[0] == '"')
			{
				++Tokenizer->At;
			}
		} break;

		default: 
		{
			if (IsAlphabetical(C))
			{
				while(IsAlphabetical(Tokenizer->At[0]) || 
					  (IsNumeric(Tokenizer->At[0]) || 
					  Tokenizer->At[0] == '_'))
				{
					++Tokenizer->At;
				}

				Token.TextLength = Tokenizer->At - Token.Text;
				Token.Type = Token_Identifier;
			}
			else if (IsNumeric(C))
			{
				// ParseNumber();
			}
			else
			{
				Token.Type = Token_Unknown;
			}
		} break;
	}

	return Token;
}

static bool TokenEquals(token Token, char* Match)
{
	char* At = Match;
	for (int i = 0; i < Token.TextLength; i++)
	{
		if (Match[i] && Token.Text[i] != Match[i]) return false;
		++At;
	}

	bool Result =  (*At == '\0');
	return Result;
}

static void ParseIntrospectionParams(tokenizer *Tokenizer)
{
	// Go through params and parse appropriately until
	// EOF or closed paren is reached
	// printf("Introspection Params: ");
	for(;;)
	{
		token Token = GetToken(Tokenizer);	
		if ((Token.Type == Token_CloseParen) ||
			(Token.Type == Token_EndOfStream))
		{
			// printf("\n");
			break;
		}
		// else printf("%.*s ", Token.TextLength, Token.Text);
	}
}

inline bool RequireToken(tokenizer *Tokenizer, token_type DesiredType)
{
	token Token = GetToken(Tokenizer);
	bool Result = Token.Type == DesiredType;
	return Result;
}

static void ParseMember(tokenizer *Tokenizer, token MemberTypeToken)
{
	// Register member type with type catalog
	RegisterTypeWithCatalog(MemberTypeToken, &TypeCatalog);

	bool Parsing = true;
	bool IsPointer = false;
	while (Parsing)
	{
		token Token = GetToken(Tokenizer);
		switch(Token.Type)
		{
			case Token_Asterisk: 
			{
				IsPointer = true;
			} break;

			case Token_Identifier: 
			{
				// printf("\t\tType: %.*s, Value: %.*s\n", MemberTypeToken.TextLength, MemberTypeToken.Text, Token.TextLength, Token.Text);
			} break;

			case Token_SemiColon: 
			case Token_EndOfStream: 
			{
				Parsing = false;
			} break;
		}
	}

}

static void ParseStruct(tokenizer *Tokenizer)
{
	// Get name of struct
	token NameToken = GetToken(Tokenizer);
	// printf("struct: %.*s\n", NameToken.TextLength, NameToken.Text);

	// Register with the type catalog
	RegisterTypeWithCatalog(NameToken, &TypeCatalog);

	// Get member variables
	if (RequireToken(Tokenizer, Token_OpenBrace))
	{
		// printf("\tMember Variables:\n");
		bool Parsing = true;
		while(Parsing)
		{
			// Gets the type of the member as a token 
			token MemberTypeToken = GetToken(Tokenizer);

			// Makes sure that we're not at end of struct
			if (MemberTypeToken.Type == Token_CloseBrace) Parsing = false;

			// Parses this member variable with member type token passed in
			else ParseMember(Tokenizer, MemberTypeToken);
		}
	}
	else
	{
		fprintf(stderr, "ERROR: Need braces for struct - %.*s", NameToken.TextLength, NameToken.Text);
	}

	// printf("\n");
}

static void ParseFunctionParams(tokenizer *Tokenizer)
{
	bool Parsing = true;
	while(Parsing)	
	{
		token Token = GetToken(Tokenizer);

		if (Token.Type == Token_CloseParen || 
			Token.Type == Token_EndOfStream)
		{
			Parsing = false;
		}
		else
		{
			// printf("%.*s ", Token.TextLength, Token.Text);
		}
	}
}

static token ParseFunctionPrepends(tokenizer *Tokenizer)
{
	// Scan ahead to see how many prepended arguments there are
	int PrependedCount = 0;
	tokenizer Scanner = *Tokenizer;
	token ScannerToken;
	token NameToken;
	while((ScannerToken = GetToken(&Scanner)).Type != Token_OpenParen)
	{
		NameToken = ScannerToken;
		PrependedCount++;
	}

	if (TokenEquals(NameToken, "inline") || 
		TokenEquals(NameToken, "const")  ||
		TokenEquals(NameToken, "static") || 
		TokenEquals(NameToken, "virtual"))
	{
		fprintf(stderr, "ERROR: Right now does not support functions having prepends after function name.");
	}

	else
	{
		// Print name of function, since it will be the last thing before paren...probably
		// printf("\n\tName: %.*s", NameToken.TextLength, NameToken.Text);
	} 

	bool Parsing = true;
	token Token;
	int Count = 0;
	while(Parsing)	
	{
		Token = GetToken(Tokenizer);

		if (Token.Type == Token_OpenParen || 
			Token.Type == Token_EndOfStream)
		{
			Parsing = false;
		}
		else if (!(TokenEquals(Token, "static") || 
				   TokenEquals(Token, "inline") || 
				   TokenEquals(Token, "const")  || 
				   TokenEquals(Token, "virtual")  || 
				   Count >= PrependedCount - 1))
		{
			// printf("\n\tReturn: %.*s ", Token.TextLength, Token.Text);

			// Register type with type catalog
			RegisterTypeWithCatalog(Token, &TypeCatalog);
		}

		Count++;
	}

	return Token;
}

static void ParseFunction(tokenizer *Tokenizer)
{
	token NextToken;

	// printf("\tFunction: ");

	// Grab prepends to function, such as return type
	// and the function name
	NextToken = ParseFunctionPrepends(Tokenizer);

	// printf("\n\tParams: ");
	if (NextToken.Type == Token_OpenParen)
	{
		ParseFunctionParams(Tokenizer);
	}

	// See if function is closed with semicolon now
	if (RequireToken(Tokenizer, Token_SemiColon))
	{
		// Do nothing
	}
	else
	{
		fprintf(stderr, "ERROR: Can only support function declarations for now.\n");
	}

	// printf("\n\n");
}

static void CheckClassForInheritance(tokenizer *Tokenizer)
{
	// For now, just scan until we get to the brace
	tokenizer Scanner = *Tokenizer;
	token ScannerToken;
	while((ScannerToken = GetToken(&Scanner)).Type != Token_OpenBrace)
	{
		*Tokenizer = Scanner;
	}

}

static void ParseClass(tokenizer *Tokenizer)
{
	// Get name of class
	token NameToken = GetToken(Tokenizer);
	// printf("class: %.*s\n", NameToken.TextLength, NameToken.Text);

	// Register class type with TypeCatalogalog
	RegisterTypeWithCatalog(NameToken, &TypeCatalog);

	// Check for inheritance
	CheckClassForInheritance(Tokenizer);

	// Get member variables
	if (RequireToken(Tokenizer, Token_OpenBrace))
	{
		bool Parsing = true;
		while(Parsing)
		{
			// Get the next token
			token NextToken = GetToken(Tokenizer);
			
			if (TokenEquals(NextToken, "public") 	|| 
				TokenEquals(NextToken, "private") 	||
				TokenEquals(NextToken, "protected"))
			{
				// Just skip these for now	
				// And eat the semicolon
				token SemiColon = GetToken(Tokenizer);
			}

			// Try and parse functions
			else if (TokenEquals(NextToken, "ENJON_FUNCTION"))
			{
				// Grab the introspection params
				if (RequireToken(Tokenizer, Token_OpenParen))
				{
					ParseIntrospectionParams(Tokenizer);
				}

				// Now parse the function
				ParseFunction(Tokenizer);
			}
			else
			{
				ParseMember(Tokenizer, NextToken);
			}

			if (NextToken.Type == Token_CloseBrace) Parsing = false;
		}
	}

	// printf("\n");
}

static void ParseIntrospection(tokenizer *Tokenizer)
{
	if (RequireToken(Tokenizer, Token_OpenParen))
	{
		// Gets params and closes the paren
		ParseIntrospectionParams(Tokenizer);

		token TypeToken = GetToken(Tokenizer);
		if (TokenEquals(TypeToken, "struct"))
		{
			ParseStruct(Tokenizer);
		}
		else if (TokenEquals(TypeToken, "class"))
		{
			ParseClass(Tokenizer);
		}
		else
		{
			fprintf(stderr, "ERROR: Introspection does not support type: %.*s\n", TypeToken.TextLength, TypeToken.Text);
		}
	}
	else
	{
		fprintf(stderr, "ERROR: Missing parantheses.\n");
	}
}

#if 0
int main(int ArgCount, char** Args)
{
	const u32 FileAmount = 1;
	char *Files[FileAmount] = {"include/TestComponent.h"};

	TypeCatalog.CurrentSize = 0;
	TypeCatalog.MaxAllocatedSize = 4;
	TypeCatalog.Data = (char**)malloc(sizeof(char*) * TypeCatalog.MaxAllocatedSize);

	for (u32 i = 0; i < FileAmount; i++)
	{
		char *FileContents = ReadFileContentsIntoString(Files[i]);

		tokenizer Tokenizer = {};
		Tokenizer.At = FileContents; 

		bool Parsing = true;
		while(Parsing)
		{
			token Token = GetToken(&Tokenizer);

			switch(Token.Type)
			{
				default: 
				{
				} break;

				case Token_Unknown: 
				{

				} break;

				case Token_Identifier: 
				{
					if (TokenEquals(Token, "Introspection"))
					{
						ParseIntrospection(&Tokenizer);
					}
					else if (TokenEquals(Token, "ENJON_STRUCT") || 
							(TokenEquals(Token, "ENJON_CLASS")))
					{
						ParseIntrospection(&Tokenizer);
					}
				} break;

				case Token_EndOfStream: 
				{
					Parsing = false;
				}break;

			}
		}

		free(FileContents);
	}

	// Make the namespace for type catalog
	printf("#ifndef ENJON_GENERATED_H\n");
	printf("#define ENJON_GENERATED_H\n\n");

	printf("/*\n");
	INDENT(1) printf("This file has been generated. All modifications will be lost.\n");
	printf("*/\n\n");

	printf("namespace TypeCatalog\n");
	printf("{\n");

		// Registered meta types enum
		INDENT(1) printf("enum MetaType\n");
		INDENT(1) printf("{\n");

			for (u32 i = 0; i < TypeCatalog.CurrentSize; i++)
			{
				INDENT(2) printf("MetaType_%s,\n", TypeCatalog.Data[i]);
			}

		INDENT(1) printf("};\n");

		// Type information as string
		printf("\n");
		INDENT(1) printf("// Type information as string\n");
		INDENT(1) printf("template <typename T>\n");
		INDENT(1) printf("inline const char* TypeName() { return \"unknown\"; }\n");
		printf("\n");
		
		for (u32 i = 0; i < TypeCatalog.CurrentSize; i++)
		{
			INDENT(1) printf("template <>\n");
			INDENT(1) printf("inline const char* TypeName<%s>() { return \"%s\"; }\n", TypeCatalog.Data[i], TypeCatalog.Data[i]);
			printf("\n");
		}

		// Get meta type
		printf("\n");
		INDENT(1) printf("// Get MetaType from class type\n");
		INDENT(1) printf("template <typename T>\n");
		INDENT(1) printf("inline MetaType GetMetaType() { return MetaType_Unknown; }\n");
		printf("\n");

		for (u32 i = 0; i < TypeCatalog.CurrentSize; i++)
		{
			INDENT(1) printf("template <>\n");
			INDENT(1) printf("inline MetaType GetMetaType<%s>() { return MetaType_%s; }\n", TypeCatalog.Data[i], TypeCatalog.Data[i]);
			printf("\n");
		}


	printf("} // TypeCatalog\n");

	printf("\n#endif\n\n");

	return 0;
}
 
#endif 

static void ParseProperty( Lexer* lexer )
{
	if ( lexer->RequireToken( TokenType::Token_OpenParen ) )
	{
		output.push_back( "Property: " );

		// Gets params and closes the paren 
		Token typeToken = lexer->GetNextToken( );

		bool parsing = true;
		while (parsing)
		{
			Token token = lexer->GetNextToken( );
			if ( token.mType == TokenType::Token_CloseParen || token.mType == TokenType::Token_EndOfStream )
			{
				parsing = false;
			}
			else
			{
				output.push_back( token.ToString( )  + "\n" );
			}
		}
	}
	else
	{
		fprintf( stderr, "ERROR: Missing parantheses.\n" );
	}
}

// Possible implementation of meta class and generated fields for reflection

#define OBJECT( type )\
friend Object;\
public:\
	virtual const MetaClass* Class( ) override;\
	virtual u32 GetTypeId() const override { return Object::GetTypeId< type >(); }		\
	virtual const char* GetTypeName() const override { return #type; } 

typedef uint32_t u32;
typedef int32_t s32;
typedef float f32; 
typedef char u8;

enum class MetaPropertyType
{
	Float,
	U32,
	S32,
	Bool,
	String
};

class Object;
class MetaProperty
{ 
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
			static_assert( std::is_base_of<Object, T>::value,
				"RegisterMetaClass:: T must inherit from Object." );

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
			static_assert( std::is_base_of<Object, T>::value,
				"GetTypeId:: T must inherit from Object." );

			static u32 typeId { GetUniqueTypeId( ) };
			return typeId;
		}
		
		/**
		*@brief
		*/
		template <typename T>
		T* Cast( )
		{
			static_assert( std::is_base_of<Object, T>::value,
				"GetTypeId:: T must inherit from Object." );

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
 
MetaClassRegistry mGlobalMetaRegistry;

// This will be generated
const MetaClass* Thing::Class( )
{
	const MetaClass* cls = mGlobalMetaRegistry.Get< Thing >( );
	if ( !cls )
	{ 
		// Construct new meta class
		cls = mGlobalMetaRegistry.RegisterMetaClass< Thing >( ); 
	}

	// Return cls
	return cls;
} 

// Template specialization for meta class construction
template <>
MetaClass* Object::ConstructMetaClass<Thing>( )
{
	MetaClass* cls = new MetaClass( );

	// Construct properties
	cls->mProperties[ "mFloatProperty" ] = MetaProperty( MetaPropertyType::Float, "mFloatProperty", ( u32 )&( ( Thing* )0 )->mFloatProperty );
	cls->mProperties[ "mUintProperty" ] = MetaProperty( MetaPropertyType::U32, "mUintProperty", ( u32 )&( ( Thing* )0 )->mUintProperty );
	cls->mProperties[ "mBoolProperty" ] = MetaProperty( MetaPropertyType::Bool, "mBoolProperty", ( u32 )&( ( Thing* )0 )->mBoolProperty );
	cls->mProperties[ "mStringProperty" ] = MetaProperty( MetaPropertyType::String, "mStringProperty", ( u32 )&( ( Thing* )0 )->mStringProperty );

	return cls;
}


struct ReflectionConfig
{
	void CollectFiles( Lexer* lexer );

	std::string mEnjonRootPath;
	std::string mProjectPath;
	std::string mConfigFilePath;
	std::string mOutputDirectory;
	std::vector< std::string > mFilesToParse;
}; 

ReflectionConfig mConfig; 
Introspection mIntrospection;

int main( int argc, char** argv )
{ 
	Thing thing( -10.5f, 6, true, "test" );
	Thing thing2( 145.543f, 5, true, "thing" );
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
			mConfig.mEnjonRootPath = String( argv[i + 1] );
		}
		
		// Set root path
		if ( arg.compare( "--project-path" ) == 0 && (i + 1) < argc )
		{
			mConfig.mProjectPath = String( argv[i + 1] );
		}
	} 

	// Grab the config file
	mConfig.mConfigFilePath = mConfig.mEnjonRootPath + "/Generator/config.cfg"; 
	std::string configFileContents = ReadFileIntoString( mConfig.mConfigFilePath.c_str( ) );

	// Create new lexer and assign config file
	Lexer* lexer = new Lexer( configFileContents ); 

	// Collect all necessary files for reflection
	mConfig.CollectFiles( lexer );

	// Iterate over collected files and parse
	for ( auto& f : mConfig.mFilesToParse )
	{
		// Get file contents
		std::string fileToParse = ReadFileIntoString( f.c_str( ) ); 

		std::cout << f << "\n";

		// Set lexer to parse contents
		lexer->SetContents( fileToParse );

		// Parse file and collect information
		mIntrospection.Parse( lexer );
	} 

	/*
	char* fileContents = ReadFileContentsIntoString( mFilePath.c_str() ); 
	Lexer* lexer = new Lexer( fileContents ); 

	std::vector<std::string> split = SplitString( mFilePath, "/" );
	std::vector<std::string> fileNameSplit = SplitString( split.back( ), ".h" );
	std::string fileName = split.at(split.size() - 2) + "_" + fileNameSplit.front( ); 
	std::string outputFileName = mOutputDirectory + "/" + fileName +  ".txt";

	std::stringstream ss;

	// Parse file contents
	bool Parsing = true;
	while (Parsing)
	{
		// Grab token from lexer
		Token token = lexer->GetToken( );

		// Switch on token type given
		switch (token.mType)
		{ 
			case TokenType::Token_Unknown:
			{ 
			} 
			break;

			case TokenType::Token_Identifier:
			{
				if ( token.Equals( "ENJON_STRUCT" ) || ( token.Equals( "ENJON_CLASS" ) ) || ( token.Equals( "ENJON_OBJECT" ) ) )
				{
					std::string tokenName = token.ToString( );

					ss << "From file: " << mFilePath.c_str( ) << "\n";
					ss << "Token equals struct or class: " << tokenName.c_str( ) << "\n";
				} 
				else if ( token.Equals( "ENJON_PROPERTY" ) )
				{
					ParseProperty( lexer );
				}
			} 
			break;

			case TokenType::Token_EndOfStream:
			{
				Parsing = false;
			}
			break; 

		}
	} 

	for ( auto& s : output )
	{
		ss << s;
	}
 
	ss.seekg( 0, std::ios::end );
	usize size = ss.tellg( ); 
	ss.seekg( 0, std::ios::beg );

	if ( size > 0 )
	{
		std::ofstream outFile( outputFileName, std::fstream::out ); 
		outFile << ss.rdbuf(); 
		outFile.close( ); 
	} 

	delete( lexer );
	*/
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

