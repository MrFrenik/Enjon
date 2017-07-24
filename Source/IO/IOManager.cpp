#include <fstream>

#include "IO/IOManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { 

	//Should be able to make this a lot faster without using ifstream
	bool IOManager::ReadFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer)
	{
		std::ifstream file(filePath, std::ios::binary);
		
		if( file.fail() )
		{
			perror(filePath.c_str());
			return false;
		}

		//Seek to the end of the file
		file.seekg(0, std::ios::end);

		//Get the file size
		std::streamoff fileSize = file.tellg();

		//Seek back to beginning to be able to read file
		file.seekg(0, std::ios::beg);
		
		//Reduce file size by any header bytes that might be present
		fileSize -= file.tellg();
		
		//Resize the buffer to the filesize
		buffer.resize((unsigned int)fileSize);

		file.read((char*)&(buffer[0]), fileSize);

		file.close();

		return true; 
	} 
}













