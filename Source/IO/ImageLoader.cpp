#include "Utils/Errors.h"
#include "IO/ImageLoader.h"
#include "IO/IOManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { 

	GLTexture ImageLoader::LoadPNG(std::string filePath, GLint MAG_PARAM, GLint MIN_PARAM, bool GenerateMips)
	{
		GLTexture texture = {}; 
		
		std::vector<unsigned char> in;
		std::vector<unsigned char> out;

		unsigned long width, height;

		if (Enjon::IOManager::ReadFileToBuffer(filePath, in) == false)
		{
			Utils::FatalError("Failed to load PNG file to buffer!");
		}

		int errorCode = DecodePNG(out, width, height, &(in[0]), in.size());
		if(errorCode != 0)
		{
			Utils::FatalError("DecodePNG failed with error: " + std::to_string(errorCode));
		}

		glGenTextures(1, &(texture.id));

		glBindTexture(GL_TEXTURE_2D, texture.id);

		glTexImage2D(
				GL_TEXTURE_2D, 
				0, 
				GL_RGBA8, 
				width, 
				height, 
				0, 
				GL_RGBA, 
				GL_UNSIGNED_BYTE, 
				&(out[0])
		);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM);
		
		if (GenerateMips) glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture.width = width;
		texture.height = height;


		return texture;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

















