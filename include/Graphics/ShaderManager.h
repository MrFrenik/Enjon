#ifndef ENJON_SHADERMANAGER_H
#define ENJON_SHADERMANAGER_H

#include <map>

#include "Graphics/GLSLProgram.h"
#include "Utils/Errors.h"

namespace Enjon { namespace ShaderManager { 

	void Init();
	void AddShader(const char* shadername, const char* vertpath, const char* fragpath);
	GLSLProgram* GetShader(const char* shadername); 
	GLSLProgram* Get(const char* shadername);
	GLuint GetUniformLocation(const char* shadername, const char* uniform);
	void UseProgram(const char* shadername);
	void UnuseProgram(const char* shadername);
	void DeleteShaders();

}}



#endif