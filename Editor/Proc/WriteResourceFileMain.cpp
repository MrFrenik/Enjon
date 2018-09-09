#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <experimental/filesystem>

namespace FS = std::experimental::filesystem;

void WriteToFile( const std::string& contents, const std::string& filePath ) 
{
	std::ofstream out( filePath );
	if ( out )
	{
		out.write( contents.c_str( ), contents.length( ) );
	} 
}

int main( int argc, char** argv )
{
	// Path to file
	std::string editorPath 		= FS::current_path().string();
	std::string resourcePath 	= editorPath + "/editor_resource.rc";

	// Responsible for writing out the resource file for the editor build
	std::string contents;
	contents += "#define IDI_ICON1 101\n";
	contents += "IDI_ICON1 ICON " + editorPath + "/enjonicon.ico"; 

	WriteToFile( contents, resourcePath );

	return 0;	
}