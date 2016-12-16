#include "IO/ResourceManager.h"
#include "IO/ObjLoader.h"
#include "Defines.h"

namespace Enjon { namespace Input { namespace ResourceManager { 

	Enjon::Graphics::TextureCache m_textureCache;

	std::unordered_map<const char*, EG::MeshInstance> MeshCache;

	Enjon::Graphics::GLTexture GetTexture(const std::string& texturePath, GLint magParams, GLint minParams) 
	{ 
		return m_textureCache.GetTexture(texturePath, magParams, minParams); 
	} 

	EG::MeshInstance* GetMesh(const char* MeshPath)
	{
		auto Search = MeshCache.find(MeshPath);
		if (Search != MeshCache.end())
		{
			return &Search->second;
		}

		// Otherwise create the mesh
		else
		{
			MeshCache[MeshPath] = LoadMeshFromFile(MeshPath);
			return &MeshCache[MeshPath];
		}
	}


}}}