#pragma once
#ifndef GENERATOR_TOKEN_H
#define GENERATOR_TOKEN_H

#include "Utils.h"

enum class TokenType
{
	Token_Unknown,
	Token_Identifier,	
	Token_OpenParen,	
	Token_CloseParen,	
	Token_LessThan,	
	Token_GreaterThan,	
	Token_String,	
	Token_Colon,	
	Token_SemiColon,	
	Token_Asterisk,	
	Token_OpenBracket,	
	Token_CloseBracket,	
	Token_OpenBrace,	
	Token_CloseBrace,
	Token_Hash,
	Token_EndOfStream,
	Token_DoubleColon,
	Token_Ampersand,
	Token_Comma,
	Token_Equal,
	Token_Number
};

class Token
{
	public:

		/*
		* @brief
		*/
		Token( );

		/*
		* @brief
		*/
		~Token( );

		/*
		* @brief
		*/
		bool Equals( const std::string& match );

		/*
		* @brief
		*/
		bool IsType( const TokenType& type );

		/*
		* @brief
		*/
		std::string ToString( );

		TokenType mType			= TokenType::Token_Unknown;
		size_t mTextLength		= 0;
		char* mText				= nullptr;	
		std::string	mTextString = "";
};

#endif
