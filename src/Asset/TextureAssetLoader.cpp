// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/TextureAssetLoader.h" 
#include "Graphics/picoPNG.h"
#include "IO/IOManager.h"

#include <GLEW/glew.h>
#include <vector>

namespace Enjon
{
	TextureAssetLoader::TextureAssetLoader()
	{ 
	}
			
	TextureAssetLoader::~TextureAssetLoader()
	{ 
	} 

	Texture* TextureAssetLoader::LoadAssetFromFile(const String& filePath, const String& name)
	{ 
		// Create new texture
		Enjon::Texture* tex = new Enjon::Texture;
		
		std::vector<u8> in;
		std::vector<u8> out;

		u32l width, height;

		// Read in file into character buffer
		if (Enjon::IOManager::ReadFileToBuffer(filePath, in) == false)
		{
			// Error check
		}

		// TODO(): Switch to using stbimage to load images
		s32 errorCode = DecodePNG(out, width, height, &(in[0]), in.size());
		if (errorCode != 0)
		{
			// Error check
		}

		// Generate texture
		// TODO(): Make this API generalized to work with DirectX as well as OpenGL
		glGenTextures(1, &(tex->mId));

		glBindTexture(GL_TEXTURE_2D, tex->mId);

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
		
		s32 MAG_PARAM = GL_LINEAR;
		s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
		b8 genMips = true;

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM);

		if (genMips)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		// Set texture attributes
		tex->mWidth = width;
		tex->mHeight = height; 

		// Add to assets with qualified name
		AddToAssets(name, tex);

		return tex;
	} 
}
