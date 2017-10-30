#pragma once
#ifndef GENERATOR_UTILS_H
#define GENERATOR_UTILS_H

#include <fstream>
#include <cstdint>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>

using u32 = uint32_t;
using s32 = int32_t;
using String = std::string;
using usize = size_t;
	
static inline u32 TagCount( const std::string& code, const std::string& tag )
{
	u32 count = 0;

	// Search for begin
	std::size_t pos = code.find( tag );
	while ( pos != std::string::npos )
	{
		count++;
		pos = code.find( tag, pos + 1 );
	}

	return count;
}

#define REPLACE_META_TAG( code, find, replace )\
for ( u32 i = 0; i < TagCount( code, find ); ++i )\
{\
	code = FindReplaceMetaTag( code, find, replace );\
} 
	
static inline std::string FindReplaceMetaTag( const std::string& code, const std::string& toFind, const std::string& replaceWith )
{
	std::string returnStr = "";

	// Search for begin
	std::size_t foundBegin = code.find( toFind );
	std::size_t findSize = toFind.length( );

	std::string subStrBefore = "";
	std::string subStrAfter = "";

	// If found, then replace and return
	if ( foundBegin != std::string::npos )
	{
		subStrBefore = code.substr( 0, foundBegin );
		subStrAfter = code.substr( foundBegin + findSize );
		return ( subStrBefore + replaceWith + subStrAfter );
	}
	// Else just return the original string
	else
	{
		return code;
	}
}

	//==================================================================================================================

static inline std::string FindReplaceAll( const std::string& code, const std::string& toFind, const std::string& replaceWith )
{
	std::string retCode = code;

	// Search for begin
	std::size_t pos = retCode.find( toFind );
	while ( pos != std::string::npos )
	{
		retCode = FindReplaceMetaTag( retCode, toFind, replaceWith );
		pos = retCode.find( toFind, pos + 1 );
	}

	return retCode;
}

template <typename T>
static inline T Clamp( const T& value, const T& min, const T& max )
{
	return value > max ? max : value < min : min : value;
}

template <typename T>
static inline T Max( const T& a, const T& b )
{
	return a >= b ? a : b;
}

static inline std::string OutputLine( const std::string& code )
{
	return code + "\n";
}

static inline std::string OutputTabbedLine( const std::string& code )
{
	return OutputLine( "\t" + code );
}
	
static inline std::vector<String> SplitString(const String& str, const String& delimiter)
{
	std::vector<String> splits;

	usize pos = 0;
	String token;
	usize last = 0;
	usize next = 0;
	while ((next = str.find(delimiter, last)) != String::npos)
	{
		token = str.substr(last, next - last);
		splits.push_back(token);
		last = next + 1;
	}

	splits.push_back(str.substr(last));

	return splits; 
}
	
static inline std::string ReadFileIntoString( const char* filePath )
{
	std::ifstream f( filePath );
	std::string str( ( std::istreambuf_iterator<char>( f ) ), std::istreambuf_iterator<char>( ) ); 
	return str;
}


static inline char* ReadFileContentsIntoString(const char* FilePath)
{
	char* Result = 0;

	FILE *File = fopen( FilePath, "r" );
	if (File)
	{
		fseek( File, 0, SEEK_END );
		size_t FileSize = ftell( File );
		fseek( File, 0, SEEK_SET );

		Result = ( char * )malloc( ( FileSize + 1 ) * sizeof( char ) );
		if ( Result )
		{
			fread( Result, sizeof( char ), FileSize, File );
			Result[FileSize] = '\0'; 
		}

		fclose( File );
	}	

	return Result;
}

static inline bool StringEqual(char* A, char* B, u32 Length)
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

inline bool IsEndOfLine( char C )
{
	return ( C == '\n' || C == '\r' );
}

inline bool IsWhiteSpace( char C )
{
	return ( ( C == ' ' ) ||
		( C == '\t' ) ||
		IsEndOfLine( C ) );
}

inline bool IsAlphabetical( char C )
{
	return ( ( C >= 'a' && C <= 'z' ) ||
		( C >= 'A' && C <= 'Z' ) );
}

inline bool IsNumeric( char C )
{
	return ( C >= '0' && C <= '9' );
} 

static inline String Replace( const String& str, const char& find, const char& with )
{
	String res = str;

	for (auto& c : res)
	{
		if (c == find)
		{
			c = with;
		}
	}

	return res;
}

static inline String ToLower( const String& str )
{
	String res = str;
	std::transform( res.begin( ), res.end( ), res.begin( ), ::tolower );
	return res;
}

#endif
