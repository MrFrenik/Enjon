#include "Graphics/ShaderManager.h"
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

namespace Enjon { namespace Graphics { namespace ShaderManager {

 	std::unordered_map<std::string, GLSLProgram*> Shaders;

	void Init()
	{
		// Add shaders
		ShaderManager::AddShader("Basic", "../shaders/basic.v.glsl", "../shaders/basic.f.glsl");
		ShaderManager::AddShader("Text","../shaders/text.v.glsl", "../shaders/text.f.glsl");
		ShaderManager::AddShader("Default","../shaders/default.v.glsl", "../shaders/default.f.glsl");
		ShaderManager::AddShader("DiffuseShader", "../shaders/diffuse.v.glsl", "../shaders/diffuse.f.glsl");
		ShaderManager::AddShader("DeferredShader", "../shaders/deferred.v.glsl", "../shaders/deferred.f.glsl");
		ShaderManager::AddShader("NormalsShader", "../shaders/normal.v.glsl", "../shaders/normal.f.glsl");
		ShaderManager::AddShader("DepthShader", "../shaders/depth.v.glsl", "../shaders/depth.f.glsl");
		ShaderManager::AddShader("NoCameraProjection", "../shaders/no_camera.v.glsl", "../shaders/no_camera.f.glsl");
		ShaderManager::AddShader("BasicLighting", "../shaders/lighting_basic.v.glsl", "../shaders/lighting_basic.f.glsl");
		ShaderManager::AddShader("Learn", "../shaders/learn.v.glsl", "../shaders/learn.f.glsl");
		ShaderManager::AddShader("Lamp", "../shaders/learn.v.glsl", "../shaders/lamplight.f.glsl");
		ShaderManager::AddShader("DefaultLighting", "../shaders/defaultLight.v.glsl", "../shaders/defaultLight.f.glsl");
		ShaderManager::AddShader("VerticalBlur", "../shaders/vertical_blur.v.glsl", "../shaders/vertical_blur.f.glsl");
		ShaderManager::AddShader("HorizontalBlur", "../shaders/horizontal_blur.v.glsl", "../shaders/horizontal_blur.f.glsl");
		ShaderManager::AddShader("GBuffer", "../shaders/deferred/deferred.v.glsl", "../shaders/deferred/deferred.f.glsl");
		ShaderManager::AddShader("DeferredLight", "../shaders/deferred/deferred_light.v.glsl", "../shaders/deferred/deferred_light.f.glsl");
		ShaderManager::AddShader("DirectionalLight", "../shaders/deferred/directional_light.v.glsl", "../shaders/deferred/directional_light.f.glsl");
		ShaderManager::AddShader("PointLight", "../shaders/deferred/point_light.v.glsl", "../shaders/deferred/point_light.f.glsl");
		ShaderManager::AddShader("Depth", "../shaders/deferred/depth.v.glsl", "../shaders/deferred/depth.f.glsl");
	}

	void AddShader(const char* shadername, const char* vertpath, const char* fragpath)
	{
		GLSLProgram* p = new GLSLProgram;
		p->CreateShader(vertpath, fragpath);
		Shaders[shadername] = p;
	}

	GLSLProgram* GetShader(const char* shadername)
	{
		auto it = Shaders.find(shadername);
		if (it != Shaders.end())
		{
			return it->second;
		}

		else
		{
			std::cout << "Nope!\n";
			// Otherwise not found
			std::string errorstr = shadername;
			Utils::FatalError("SHADERMANAGER::GETSHADER::SHADER_NOT_FOUND::" + errorstr);
			return NULL;
		}	
	} 
	
	GLuint GetUniformLocation(const char* shadername, const char* uniformname)
	{
		return GetShader(shadername)->GetUniformLocation(uniformname); 
	} 

	void UseProgram(const char* shadername)
	{
		GetShader(shadername)->Use(); 
	}

	void UnuseProgram(const char* shadername)
	{
		GetShader(shadername)->Unuse();
	}
	
	void DeleteShaders()
	{ 

	}

}}}