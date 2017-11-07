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
		
void Lexer::SetContents( const std::string& newContents, const std::string& contentsPath )
{
	mContentsString = newContents;
	mContents = &mContentsString[ 0 ];
	mAt = mContents;
	mCurrentToken = Token( );
	mContentsPath = contentsPath;
}

void Lexer::SetToken( Token token )
{
	mCurrentToken = token; 
	mAt = token.mText;
}
		
Token Lexer::PeekAtNextToken( )
{ 
	// Grab next token to return
	Token nextToken = GetNextToken( false ); 

	return nextToken;
}

bool Lexer::ContinueTo( TokenType type )
{
	Token token = GetNextToken( );

	// Continue to search for type or hit end of stream
	while ( !token.IsType( type ) && !token.IsType( TokenType::Token_EndOfStream ) )
	{
		token = GetNextToken( );
	}

	bool retVal = !token.IsType( TokenType::Token_EndOfStream );
	return retVal;
}
		
bool Lexer::ContinueToIdentifier( const std::string& identifier )
{
	Token token = GetNextToken( );

	// Continue to search for identifier or hit end of stream
	while ( !token.Equals( identifier ) && !token.IsType( TokenType::Token_EndOfStream ) )
	{
		token = GetNextToken( );
	}

	// If EOS hit, then didn't find identifier
	return !token.IsType( TokenType::Token_EndOfStream ); 
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

Token Lexer::GetNextToken( bool advance )
{
	EatAllWhiteSpace( );

	Token token = { };
	token.mTextLength = 1;
	token.mText = mAt;
	char C = mAt[ 0 ];

	if ( advance )
	{
		++mAt; 
	}

	switch (C)
	{
		case '(': {token.mType = TokenType::Token_OpenParen; }		break;
		case ')': {token.mType = TokenType::Token_CloseParen; } 	break;
		case '<': {token.mType = TokenType::Token_LessThan; } 		break;
		case '>': {token.mType = TokenType::Token_GreaterThan; } 	break;
		case ';': {token.mType = TokenType::Token_SemiColon; }		break;
		case '*': {token.mType = TokenType::Token_Asterisk; }		break;
		case '&': {token.mType = TokenType::Token_Ampersand; }		break;
		case '{': {token.mType = TokenType::Token_OpenBrace; }		break;
		case '}': {token.mType = TokenType::Token_CloseBrace; }		break;
		case '[': {token.mType = TokenType::Token_OpenBracket; }	break;
		case ']': {token.mType = TokenType::Token_CloseBracket; }	break;
		case '\0':{token.mType = TokenType::Token_EndOfStream; } 	break;
		case '#': {token.mType = TokenType::Token_Hash; } 			break;
		case ',': {token.mType = TokenType::Token_Comma; } 			break;
		case '=': {token.mType = TokenType::Token_Equal; } 			break;
		
		case ':': 
		{
			// Search for double colon first
			token.mType = TokenType::Token_Colon; 

			if ( !advance )
			{
				if ( mAt[ 1 ] && mAt[ 1 ] == ':' )
				{ 
					token.mType = TokenType::Token_DoubleColon;
					token.mTextLength = 2;
				}
			}
			else
			{
				if ( mAt[ 0 ] && mAt[ 0 ] == ':' )
				{
					token.mType = TokenType::Token_DoubleColon;
					token.mTextLength = 2;
				}

				++mAt; 
			}
		} 			
		break;

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
			if (IsAlphabetical( C ) && C != '-' )
			{
				while (IsAlphabetical( mAt[0] ) || ( IsNumeric( mAt[0] ) || mAt[0] == '_' ))
				{
					++mAt;
				}

				token.mTextLength = mAt - token.mText;
				token.mType = TokenType::Token_Identifier;
			}
			else if (IsNumeric( C ) || C == '-' )
			{
				u32 numDecimals = 0;
				while ( IsNumeric( mAt[ 0 ] ) || ( mAt[ 0 ] == '.' && numDecimals == 0 ) || mAt[0] == 'f' )
				{
					// Grab decimal
					numDecimals = mAt[ 0 ] == '.' ? numDecimals++ : numDecimals;

					// Increment
					++mAt;
				}

				token.mTextLength = mAt - token.mText;
				token.mType = TokenType::Token_Number;
			}
			else
			{
				token.mType = TokenType::Token_Unknown;
			}

		} break;
	}

	// Get text string for debugging
	token.mTextString = token.ToString( );

	if ( advance )
	{
		// Set current token
		mCurrentToken = token; 
	}
	// Otherwise reset
	else
	{
		mAt = token.mText;
	}

	return token; 
} 

Token Lexer::GetCurrentToken( )
{
	return mCurrentToken;
}
		
bool Lexer::RequireToken( TokenType type, bool advance )
{ 
	// Get token
	Token token = advance ? GetNextToken( ) : GetCurrentToken( ); 

	// Return whether or not the types match
	return ( token.mType == type );
}


