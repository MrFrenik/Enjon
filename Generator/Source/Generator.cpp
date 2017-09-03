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
#include <functional> 

struct Thing
{
	Thing( )
	{ 
	}

	~Thing( )
	{ 
	}

	void SetRadius( float radius )
	{
		mRadius = radius;
	}

	float GetRadius( ) const
	{
		return mRadius;
	}

	float mRadius; 
}; 

/*
* Hold a heterogenuous group of functions
std::vector< MetaFunction > mFunctions;
*/

class MetaFunction
{
	
};
 
int main( int argc, char** argv )
{ 
	Thing thing; 
	thing.mRadius = 5.0f;

	std::function <void( Thing*, float )> setRad = &Thing::SetRadius; 
	std::function <float( Thing* )> getRad = &Thing::GetRadius; 

	float rad = getRad( &thing );

	ReflectionConfig mConfig; 
	Introspection mIntrospection;
	mIntrospection.Initialize( );

	// Parse arguments and place into config
	for ( s32 i = 0; i < argc; ++i )
	{
		std::string arg = std::string( argv[i] ); 

		// Set root path

		if ( arg.compare( "--enjon-path" ) == 0 && (i + 1) < argc )
		{
			mConfig.mEnjonRootPath = FindReplaceMetaTag( String( argv[i + 1] ), "/Generator/..", "" );
		}
		
		// Set root path
		if ( arg.compare( "--project-path" ) == 0 && (i + 1) < argc )
		{
			mConfig.mProjectPath = String( argv[i + 1] );
		}
	} 

	// Grab the config file
	mConfig.mConfigFilePath = mConfig.mEnjonRootPath + "/Generator/config.cfg"; 
	mConfig.mOutputDirectory = mConfig.mEnjonRootPath + "/Build/Generator/Intermediate";
	mConfig.mLinkedDirectory = mConfig.mEnjonRootPath + "/Build/Generator/Linked";

	std::string configFileContents = ReadFileIntoString( mConfig.mConfigFilePath.c_str( ) );

	// Create new lexer and assign config file
	Lexer* lexer = new Lexer( configFileContents ); 

	// Collect all necessary files for reflection
	mConfig.CollectFiles( lexer );

	// Iterate over collected files and parse
	for ( auto& f : mConfig.mFilesToParse )
	{ 
		std::vector< std::string > splits = SplitString( f, "/" );

		// Fix this with an actual list of files to parse instead of ALL engine headers
		if ( splits.back( ).compare( "Object.h" ) == 0 )
		{
			continue;
		}
		
		// Output the file being generated
		std::cout << "Generating reflecton for " << splits.back() <<  "\n";

		// Get file contents
		std::string fileToParse = ReadFileIntoString( f.c_str( ) ); 

		// Set lexer to parse contents
		lexer->SetContents( fileToParse, f ); 

		// Parse file and collect information
		mIntrospection.Parse( lexer ); 
	} 

	// Write classes to file
	mIntrospection.Compile( mConfig ); 

	// Link all classes into one generated file
	mIntrospection.Link( mConfig ); 
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

