#include "IO/ResourceManager.h"

namespace Enjon { namespace Input { namespace ResourceManager { 

	Enjon::Graphics::TextureCache m_textureCache;

	Enjon::Graphics::GLTexture GetTexture(const std::string& texturePath) 
	{ 
		return m_textureCache.GetTexture(texturePath); 
	} 

}}}