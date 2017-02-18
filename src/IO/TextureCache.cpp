#include <iostream>

#include "IO/TextureCache.h"
#include "IO/ImageLoader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {


	TextureCache::TextureCache()
	{
	}

	TextureCache::~TextureCache()
	{
	}

	GLTexture& TextureCache::GetTexture(std::string texturePath, GLint magParams, GLint minParams, bool genmips)
	{
		//Set mit to map iterator; lookup texture to see if it's in map
		auto mit = m_textureMap.find(texturePath);

		//Check to see if it's not in map
		if(mit == m_textureMap.end())
		{
			GLTexture newTexture = ImageLoader::LoadPNG(texturePath, magParams, minParams, genmips);
			
			//Insert it into map
			m_textureMap.insert(make_pair(texturePath, newTexture));

			return newTexture;
		}
		
		return mit->second; 
	}
}}