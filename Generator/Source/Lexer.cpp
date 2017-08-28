#include "Lexer.h"
		
Lexer::Lexer( const std::string& contents )
{
	mContentsString = contents;
	mContents = &mContentsString[0];
	mAt = mContents;
	mCurrentToken = Token( );
}

Lexer::~Lexer( )
{
}
		
void Lexer::SetContents( const std::string& newContents )
{
	mContentsString = newContents;
	mContents = &mContentsString[ 0 ];
	mAt = mContents;
	mCurrentToken = Token( );
}

void Lexer::EatAllWhiteSpace( )
{
	for ( ; ; )
	{
		if ( IsWhiteSpace( mAt[0] ) )
		{
			++mAt;
		}

		else if ( ( mAt[0] == '/' ) && ( mAt[1] ) && ( mAt[1] == '/' ))
		{
			mAt += 2;
			while ( mAt[0] && !IsEndOfLine( mAt[0] ) )
			{
				++mAt;
			}
		}

		else if ( ( mAt[0] == '/' ) && ( mAt[1] ) && ( mAt[1] == '*' ) )
		{
			mAt += 2;
			while ( mAt[0] && mAt[1] && !( mAt[0] == '*' && mAt[1] == '/' ) )
			{
				++mAt;
			}
			if ( mAt[0] == '*' )
			{
				++mAt;
			}
		}

		else
		{
			break;
		}
	}
}

Token Lexer::GetNextToken( )
{
	EatAllWhiteSpace( );

	Token token = { };
	token.mTextLength = 1;
	token.mText = mAt;
	char C = mAt[0];
	++mAt;

	switch (C)
	{
		case '(': {token.mType = TokenType::Token_OpenParen; }		break;
		case ')': {token.mType = TokenType::Token_CloseParen; } 	break;
		case '<': {token.mType = TokenType::Token_LessThan; } 		break;
		case '>': {token.mType = TokenType::Token_GreaterThan; } 	break;
		case ':': {token.mType = TokenType::Token_Colon; } 			break;
		case ';': {token.mType = TokenType::Token_SemiColon; }		break;
		case '*': {token.mType = TokenType::Token_Asterisk; }		break;
		case '{': {token.mType = TokenType::Token_OpenBrace; }		break;
		case '}': {token.mType = TokenType::Token_CloseBrace; }		break;
		case '[': {token.mType = TokenType::Token_OpenBracket; }	break;
		case ']': {token.mType = TokenType::Token_CloseBracket; }	break;
		case '\0':{token.mType = TokenType::Token_EndOfStream; } 	break;
		case '#': {token.mType = TokenType::Token_Hash; } 			break;

		case '/':
		{ 
		} break;

		case '"':
		{
			token.mText = mAt;

			while (mAt[0] &&
				mAt[0] != '"')
			{
				if (mAt[0] == '\\' &&
					mAt[1])
				{
					++mAt;
				}
				++mAt;
			}

			token.mTextLength = mAt - token.mText;
			token.mType = TokenType::Token_String;

			if (mAt[0] == '"')
			{
				++mAt;
			}

		} break;

		default:
		{
			if (IsAlphabetical( C ))
			{
				while (IsAlphabetical( mAt[0] ) || ( IsNumeric( mAt[0] ) || mAt[0] == '_' ))
				{
					++mAt;
				}

				token.mTextLength = mAt - token.mText;
				token.mType = TokenType::Token_Identifier;
			}
			else if (IsNumeric( C ))
			{
				// ParseNumber();
			}
			else
			{
				token.mType = TokenType::Token_Unknown;
			}

		} break;
	}

	// Get text string for debugging
	token.mTextString = token.ToString( );

	// Set current token
	mCurrentToken = token; 

	// Return current token
	return mCurrentToken; 
} 

Token Lexer::GetCurrentToken( )
{
	return mCurrentToken;
}
		
bool Lexer::RequireToken( TokenType type, bool advance )
{ 
	Token token = advance ? GetNextToken( ) : GetCurrentToken( ); 
	bool res = token.mType == type;
	return res; 
}


