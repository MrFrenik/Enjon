#ifndef ENJON_MODEL_ASSET_H
#define ENJON_MODEL_ASSET_H

#include <Defines.h>
#include <Graphics/GLSLProgram.h>
#include <Graphics/Transform.h>
#include <IO/ResourceManager.h>
#include <Math/Mat4.h>

namespace Enjon { namespace Graphics { 

	struct ModelAsset
	{
		EG::GLSLProgram* Shader;
		EG::GLTexture Texture;

		GLuint VAO;
		GLuint VBO;
		GLuint IBO;

		GLenum DrawType;
		GLint DrawStart;
		GLint DrawCount;
	};

	struct ModelInstance
	{
		ModelAsset* Asset;
		EG::Transform Transform;
	};

}}

#endif