#ifndef ENJON_TOKENIZER_H
#define ENJON_TOKENIZER_H

namespace Enjon { namespace Utils { 

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
		Token_Float,
		Token_Int,
		Token_Minus,
		Token_ForwardSlash,
		Token_BackwardSlash
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
	
	char* NullTerminatedStringFromToken(token* Token)
	{
		char* ReturnString = (char*)malloc(Token->TextLength + 1);
		int i;
		for (i = 0; i < Token->TextLength; i++)
		{
			ReturnString[i] = Token->Text[i];
		}

		ReturnString[i] = '\0';

		return ReturnString;
	}

	bool inline TokenizedStringsEqual(char* A, char* B, u32 Length)
	{
		u32 i;
		for (i = 0; i < Length; i++)
		{
			if (!(A[i] && 
				  B[i] &&
				  A[i] == B[i])) return false;
		}
		return (B[i] == 0);
	}

	inline bool IsEndOfLine(char C)
	{
		return (C == '\n' || C == '\r');
	}

	inline bool IsWhiteSpace(char C)
	{
		return ((C == ' ')  ||
				(C == '\t') ||
				IsEndOfLine(C));
	}

	inline bool IsAlphabetical(char C)
	{
		return ((C >= 'a' && C <= 'z' ) ||
				(C >= 'A' && C <= 'Z'));
	}

	inline bool IsNumeric(char C)
	{

		return (C >= '0' && C <= '9');	
	}

	inline void ParseNumber(tokenizer* Tokenizer, token* Token)
	{
		// Check to see whether or not is a floating point number
		uint32_t NumberOfFloatingPoints = 0;
		bool IsFloat = false;
		bool IsNegative = false;

		// Check whether or not the number is negative
		if (*(Tokenizer->At - 1) == '-' ) 	{ IsNegative = true; }

		// Set token's starting text pointer
		Token->Text = Tokenizer->At - 1;

		while(IsNumeric(Tokenizer->At[0]) || Tokenizer->At[0] == '.')
		{
			if (Tokenizer->At[0] == '.')
			{
				// Check to see whether or 
				NumberOfFloatingPoints++;
				IsFloat = true;

				if (NumberOfFloatingPoints > 1) break;	
			}

			++Tokenizer->At;
		}

		// Set text
		Token->TextLength = Tokenizer->At - Token->Text;

		// If we've run into too many floating points, then return unknown
		if (NumberOfFloatingPoints > 1)  { Token->Type = Token_Unknown; return; }

		// Otherwise if float set that type
		else if (IsFloat) Token->Type = Token_Float;
		// Otherwise if int set that type
		else Token->Type = Token_Int;

		// Debug print
		// if (IsNegative) printf("Negative:  %.*s\n", Token->TextLength, Token->Text);
		// else 			printf("Positive:  %.*s\n", Token->TextLength, Token->Text);
	}

	static inline void EatAllWhiteSpace(tokenizer *Tokenizer)
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

	static inline token GetToken(tokenizer *Tokenizer)
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
			case '/': 	{Token.Type = Token_ForwardSlash; } break;
			case '\0': 	{Token.Type = Token_EndOfStream; } 	break;

			case '-': 
			{
				if (Tokenizer->At[0] && IsNumeric(Tokenizer->At[0]))
				{
					ParseNumber(Tokenizer, &Token);
				}
				else if (Tokenizer->At[0] && IsWhiteSpace(Tokenizer->At[0]))
				{
					Token.Type = Token_Minus;
				}
				else
				{
					Token.Type = Token_Unknown;
				}

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
					// // Check to see whether or not is a floating point number
					// uint32_t NumberOfFloatingPoints = 0;
					// bool IsFloat = false;

					// // Set text
					// Token.Text = Tokenizer->At - 1;

					// while(IsNumeric(Tokenizer->At[0]) || Tokenizer->At[0] == '.')
					// {
					// 	if (Tokenizer->At[0] == '.')
					// 	{
					// 		// Check to see whether or 
					// 		NumberOfFloatingPoints++;
					// 		IsFloat = true;

					// 		if (NumberOfFloatingPoints > 1) break;	
					// 	}

					// 	++Tokenizer->At;
					// }

					// Token.TextLength = Tokenizer->At - Token.Text;
					// if (IsFloat) Token.Type = Token_Float;
					// else if (NumberOfFloatingPoints > 1) Token.Type = Token_Unknown;
					// else Token.Type = Token_Int;
					// printf("Number: %.*s\n", Token.TextLength, Token.Text);
					ParseNumber(Tokenizer, &Token);
				}

				else
				{
					Token.Type = Token_Unknown;
				}

			} break;
		}

		return Token;
	}

	static inline bool TokenEquals(token Token, char* Match)
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
	
	inline bool RequireToken(tokenizer *Tokenizer, token_type DesiredType)
	{
		token Token = GetToken(Tokenizer);
		bool Result = Token.Type == DesiredType;
		return Result;
	}
}}

#endif