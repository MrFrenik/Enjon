#ifndef ENJON_FILEUTILS_H
#define ENJON_FILEUTILS_H

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include "Utils/json.h"
#include "Defines.h"
#include "System/Types.h"

namespace Enjon { namespace Utils 
{ 
	inline std::string read_file_sstream(const char* filePath)
	{
		// Get and open input file
		std::ifstream inFile;
		inFile.open(filePath);

		// Stream from file into stringstream
		std::stringstream strStream;
		strStream << inFile.rdbuf();

		// Close the file stream
		inFile.close();

		// Return contents in string
		return strStream.str();
	}

		inline std::string read_file(const char* filePath)
	{
		//Create file pointer to filepath
		//FILE* file = fopen(filePath, "rt"); 
		FILE* file;
		fopen_s(&file, filePath, "rt"); 

		//Seek to end of file
		fseek(file, 0, SEEK_END);

		//Get length of file
		unsigned long length = ftell(file);

		//Create buffer to be read into
		char* data = new char[length + 1];

		//Set data to 0, since ftell is broken...
		memset(data, 0, length + 1);

		//Seek back to beginning of file
		fseek(file, 0, SEEK_SET);

		//Read into data
		fread(data, 1, length, file);

		//close file
		fclose(file);

		//Create return string
		std::string result(data);

		//Free data
		delete[] data;

		return result; 
	}

	//=========================================================================================================================

	static inline Enjon::String OutputLine( const Enjon::String& line )
	{
		return line + "\n";
	}

	//=========================================================================================================================

	static inline Enjon::String OutputTabbedLine( const Enjon::String& line )
	{
		return "\t" + line + "\n";
	}

	//=========================================================================================================================

	static inline Enjon::String ParseFromTo( const Enjon::String& begin, const Enjon::String& end, const Enjon::String& src, bool inclusive = true )
	{
		Enjon::String returnStr = "";

		// Search for line to match
		std::size_t foundBegin = src.find( begin );
		std::size_t foundEnd = src.find( end );

		if ( !inclusive )
		{
			foundBegin += begin.length( );
		}
		
		// Found
		if ( foundBegin != std::string::npos && foundEnd != std::string::npos )
		{
			// Substring length
			std::size_t length = foundEnd - foundBegin; 

			// Return that substring
			return src.substr( foundBegin, length ); 
		} 

		// Didn't find anything, so return empty string
		return returnStr;
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

	static inline String Remove( const String& str, const char& find )
	{
		String res = "";

		for ( auto& c : str )
		{
			if ( c != find )
			{
				res.push_back( c );
			}
		}

		return res;
	}
		
	static inline String Replace(const String& str, const char& find, const char& with)
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

	inline bool HasFileExtension( const String& file, const String& extension )
	{
		return ( SplitString( file, "." ).back( ).compare( extension ) == 0 );
	} 

	static inline bool FileExists( const String& filePath )
	{
		std::ifstream f( filePath.c_str( ) );
		return f.good( );
	}
	
	static inline String ToLower(const String& str)
	{
		String res = str;
		std::transform(res.begin(), res.end(), res.begin(), ::tolower);
		return res;
	}

	static inline char* ReadFileContentsIntoString(const char* FilePath)
	{
		char* Result = 0;

		FILE *File = fopen(FilePath, "r");
		if (File)
		{
			fseek(File, 0, SEEK_END);
			size_t FileSize = ftell(File);
			fseek(File, 0, SEEK_SET);

			Result = (char *)malloc(FileSize + 1);
			fread(Result, FileSize, 1, File);
			Result[FileSize] = '\0';

			fclose(File);
		}	

		return Result;
	}


	inline bool save_to_json(const char* filePath, nlohmann::json& content, uint32_t format_width = 4)
	{
		std::ofstream myfile(filePath);

		if (myfile.fail()) return false;

		myfile << content.dump(format_width);
		myfile.close();

		return true;
	}

	inline void ConvertToWchar(char* buffer, wchar_t* wcstring)
	{
		 size_t origsize = strlen(buffer) + 1;
		//const size_t newsize = 100;
		size_t convertedChars = 0;
		//wchar_t string[newsize];
		mbstowcs_s(&convertedChars, wcstring, origsize, buffer, _TRUNCATE);
		//wcscpy_s(wcstring, string);
	}
	
	inline int convert_buffer_to_int(char* buffer, int digits)
	{
		int d = digits - 1, num = 0, index = 0, mult = 1;
						
		while (d > 0)
		{
			mult *= 10;
			d--;
		}

		while (buffer[index] != '\0')
		{
			int n = buffer[index] - '0';
			num += mult * n;
			mult /= 10;
			index++;
		}

		return num;
	}

	inline float convert_buffer_to_float(char* buffer, int size, int decimalIndex)
	{
		int d = size - 2, multleft = 1, decimal = decimalIndex, i = 0;
		float lod = 0.0f, rod = 0.0f, multright = 0.1f; 

		// Get digits to left
		while (i < decimalIndex - 1)
		{
			multleft *= 10;	
			i++;
		} 

		// Get first number
		i = 0;
		while (buffer[i] != '.')
		{
			float n = (float)(buffer[i] - '0');
			lod += (float)(multleft * n);
			multleft /= 10;
			i++;
		}

		// Get past decimal
		i++;

		// Get second number
		while (buffer[i] != '\0')
		{
			float n = (float)(buffer[i] - '0');
			rod += (float)(multright * n);
			i++;
			multright /= 10.0f;
		}

		return lod + rod; 
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

	//==================================================================================================================

	static inline void WriteToFile( const String& contents, const String& filePath )
	{
		std::ofstream out( filePath );
		if ( out )
		{
			out.write( contents.c_str( ), contents.length( ) );
		} 
	}

	//==================================================================================================================

}} 

#endif