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

int main( int argc, char** argv )
{ 
	ReflectionConfig mConfig; 
	Introspection mIntrospection;
	mIntrospection.Initialize( );

	// Parse arguments and place into config
	for ( s32 i = 0; i < argc; ++i )
	{
		std::string arg = std::string( argv[i] ); 

		// Set root path

		if ( arg.compare( "--target-path" ) == 0 && (i + 1) < argc )
		{
			mConfig.mRootPath = String( argv[ i + 1 ] );
			//mConfig.mRootPath = FindReplaceMetaTag( String( argv[i + 1] ), "/Generator/..", "" );
		} 
	} 

	// Grab the config file
	mConfig.mConfigFilePath = mConfig.mRootPath + "Build/Generator/reflection.cfg"; 
	mConfig.mOutputDirectory = mConfig.mRootPath + "Build/Generator/Intermediate";
	mConfig.mLinkedDirectory = mConfig.mRootPath + "Build/Generator/Linked";

	std::string configFileContents = ReadFileIntoString( mConfig.mConfigFilePath.c_str( ) );

	// Create new lexer and assign config file
	Lexer* lexer = new Lexer( configFileContents ); 

	// Collect all necessary files for reflection
	mConfig.CollectFiles( lexer );

	std::cout << "Starting Reflection Generation on: " << mConfig.mConfigFilePath << "\n";

	// If not an engine project, then set id to last used engine id (for now just a large number)
	if ( mConfig.mProjectName.compare( "Enjon" ) != 0 )
	{
		mIntrospection.SetTypeID( 200 );
	}

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
		std::cout << "Generating reflecton data for " << splits.back() <<  "\n";

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
					if ( nextToken.Equals( "target_dir" ) )
					{
						if ( lexer->RequireToken( TokenType::Token_String, true ) )
						{
							mRootPath = lexer->GetCurrentToken( ).ToString( );
						}
					}
					if ( nextToken.Equals( "engine_dir" ) )
					{
						if ( lexer->RequireToken( TokenType::Token_String, true ) )
						{
							mEnginePath = lexer->GetCurrentToken( ).ToString( );
						}
					}
					if ( nextToken.Equals( "files" ) )
					{
						// Advance next token - if is string, then is file name
						while ( lexer->RequireToken( TokenType::Token_String, true ) )
						{
							mFilesToParse.push_back( lexer->GetCurrentToken( ).ToString( ) ); 
						}
					} 
					if ( nextToken.Equals( "additional_includes" ) )
					{
						// Advance next token - if is string, then is file name
						while ( lexer->RequireToken( TokenType::Token_String, true ) )
						{
							mAdditionalIncludes.push_back( lexer->GetCurrentToken( ).ToString( ) ); 
						} 
					}
					if ( nextToken.Equals( "project_name" ) )
					{
						if ( lexer->RequireToken( TokenType::Token_String, true ) )
						{
							mProjectName = lexer->GetCurrentToken( ).ToString( );
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

