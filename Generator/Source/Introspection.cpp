#include "Introspection.h"
#include "Token.h"
#include "Lexer.h"

#include <iostream>

//=================================================================================================

Introspection::Introspection( )
{ 
}

//=================================================================================================
		
Introspection::~Introspection( )
{ 
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
				if ( token.Equals( "ENJON_STRUCT" ) || ( token.Equals( "ENJON_CLASS" ) ) || ( token.Equals( "ENJON_OBJECT" ) ) )
				{
					std::string tokenName = token.ToString( ); 
					std::cout << tokenName << "\n";
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