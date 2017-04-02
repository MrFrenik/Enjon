#include "Token.h"

Token::Token() 
{ 
} 

Token::~Token( )
{ 
}

bool Token::Equals( char* match )
{
	char* at = match;

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
		
std::string Token::GetAsString( )
{
	std::string tokenName = "";
	for ( u32 i = 0; i < mTextLength; ++i )
	{
		tokenName += mText[i];
	}

	return tokenName; 
}

