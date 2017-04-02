#pragma once
#ifndef GENERATOR_LEXER_H
#define GENERATOR_LEXER_H

#include "Token.h"
#include "Utils.h"

class Lexer
{
	public:

		Lexer( );

		Lexer( char* contents );

		~Lexer( ); 

		void EatAllWhiteSpace( ); 

		Token GetToken( ); 

		bool RequireToken( TokenType type );
	
	public: 
		char* mAt = nullptr;

	private:	
		char* mContents = nullptr;
};

#endif
