#include "Graphics/ShaderManager.h"
#include <stdlib.h>
#include <string.h>

namespace Enjon { namespace Graphics { namespace ShaderManager {

	// TODO(John): Switch to using a hash map(unordered map)
	typedef struct
	{
		const char* name;
		GLSLProgram* shader;
	}ShaderPair;

	typedef struct
	{
		ShaderPair* pairs;
		int size;
		int capacity;
	} Shaders;
 
 	Shaders shaders;
 	ShaderPair* current_shader;

	void Init()
	{
		/////////////////
		// ADD SHADERS //
		/////////////////

		// Init shaders struct
		shaders.size = 0;
		shaders.capacity = 10;
		shaders.pairs = (ShaderPair*)malloc(shaders.capacity * sizeof* shaders.pairs);

		// Add shaders
		ShaderManager::AddShader("Basic", "../shaders/basic.v.glsl", "../shaders/basic.f.glsl");
		ShaderManager::AddShader("Text","../shaders/text.v.glsl", "../shaders/text.f.glsl");
		ShaderManager::AddShader("FrameBuffer", "../shaders/framebuffers.v.glsl", "../shaders/framebuffers.f.glsl");
		ShaderManager::AddShader("ScreenShader", "../shaders/screenshader.v.glsl", "../shaders/screenshader.f.glsl");
		ShaderManager::AddShader("NormalShader", "../shaders/framebuffers.v.glsl", "../shaders/normal.f.glsl");
		ShaderManager::AddShader("DepthShader", "../shaders/depth.v.glsl", "../shaders/depth.f.glsl");
		ShaderManager::AddShader("NoCameraProjection", "../shaders/no_camera.v.glsl", "../shaders/no_camera.f.glsl");
	}

	void ResizeShaders(int capacity)
	{
		ShaderPair* pairs = NULL;
		pairs = (ShaderPair*)realloc(pairs, capacity * sizeof *pairs);
		if (pairs)
		{
			shaders.pairs = pairs;
			shaders.capacity = capacity;
		}
	}

	void AddShader(const char* shadername, const char* vertpath, const char* fragpath)
	{
		// Resize if necessary
		if (shaders.size == shaders.capacity) ResizeShaders(shaders.capacity * 2);
	
		// Get variables	
		int i, *size = &shaders.size, found = 0;
		ShaderPair* pairs = shaders.pairs;


		// Check to make sure that shader doesn't already exist
		for (i = 0; i < *size; i++)
		{
			const char* name = pairs[i].name;
			if (strcmp(name, shadername) == 0) found = 1;
		}
		
		if (!found)	
		{
			GLSLProgram* program = new GLSLProgram;
			program->CreateShader(vertpath, fragpath);
			pairs[shaders.size++] = {shadername, program};
			current_shader = &pairs[shaders.size];
		}
	}

	GLSLProgram* GetShader(const char* shadername)
	{
		// NOTE(John): Could make this faster by sorting the map by name for binary searches
		// Search for shader
		if (shaders.size > 0)
		{
			ShaderPair* pairs = shaders.pairs;
			for (int i = 0; i < shaders.size; i++)
			{
				const char* name = pairs[i].name;
				if (strcmp(name, shadername) == 0) 
				{
					current_shader = &pairs[i];
					return pairs[i].shader;
				}
			}
		}

		// Otherwise not found
		std::string errorstr = shadername;
		Utils::FatalError("SHADERMANAGER::GETSHADER::SHADER_NOT_FOUND::" + errorstr);

		return NULL;
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