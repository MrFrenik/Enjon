#pragma once
#ifndef GENERATOR_LEXER_H
#define GENERATOR_LEXER_H

#include "Token.h"
#include "Utils.h"

class Lexer
{
	public:

		Lexer( );

		Lexer( const std::string& contents );

		~Lexer( ); 

		void EatAllWhiteSpace( ); 

		Token GetNextToken( bool advance = true ); 

		Token PeekAtNextToken( );

		bool RequireToken( TokenType type, bool advance = false );

		Token GetCurrentToken( ); 

		void SetToken( Token token );

		void SetContents( const std::string& newContents, const std::string& contentsPath = "" );

		std::string GetContentsPath( ) const { return mContentsPath; }

		bool ContinueTo( TokenType type );

		bool ContinueToIdentifier( const std::string& identifier );
	
	public: 
		char* mAt = nullptr;

	private:	
		std::string mContentsString;
		std::string mContentsPath;
		char* mContents = nullptr;
		Token mCurrentToken;
};

#endif
