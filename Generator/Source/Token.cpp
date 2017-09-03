#include "Token.h"

Token::Token() 
{ 
} 

Token::~Token( )
{ 
}

bool Token::Equals( const std::string& match )
{
	// Early out
	if ( match.length( ) != mTextLength )
	{
		return false;
	}

	const char* at = &match[0];

	for (s32 i = 0; i < mTextLength; i++)
	{
		if (match[i] && mText[i] != match[i])
		{
			return false;
		}

		++at;
	}

	bool res = ( *at == '\0' );

	return res;
}
		
bool Token::IsType( const TokenType& type )
{
	return ( mType == type );
}
		
std::string Token::ToString( )
{
	std::string tokenName = "";
	for ( u32 i = 0; i < mTextLength; ++i )
	{
		tokenName += mText[i];
	}

	return tokenName; 
}

