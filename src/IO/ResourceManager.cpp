#include "IO/ResourceManager.h"
#include "IO/ObjLoader.h"
#include "Defines.h"

namespace Enjon { namespace Input { namespace ResourceManager { 

	Enjon::Graphics::TextureCache m_textureCache;
	std::unordered_map<std::string, EG::Mesh> MeshCache;

	Enjon::Graphics::GLTexture& GetTexture(const std::string& texturePath, GLint magParams, GLint minParams, bool genmips) 
	{ 
		return m_textureCache.GetTexture(texturePath, magParams, minParams, genmips); 
	} 

	EG::Mesh* GetMesh(const std::string& MeshPath)
	{
		auto Search = MeshCache.find(MeshPath);
		if (Search != MeshCache.end())
		{
			return &Search->second;
		}

		// Otherwise create the mesh
		else
		{
			MeshCache[MeshPath] = LoadMeshFromFile(MeshPath.c_str());
			return &MeshCache[MeshPath];
		}
	}


}}}