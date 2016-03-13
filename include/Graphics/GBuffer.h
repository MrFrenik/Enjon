#ifndef GBUFFER_H
#define GBUFFER_H

#include "System/Types.h"
#include "GLEW/glew.h"

namespace Enjon { namespace Graphics { 

	class GBuffer
	{
	public:
		GBuffer();
		~GBuffer();

		bool Create(uint32 Width, uint32 Height);

		void Bind();
		void Unbind();

		uint32 Width;
		uint32 Height;
		GLuint FBO;

		GLuint Diffuse;
		GLuint Specular;
		GLuint Normals;
		GLuint Depth;
	};

}}

#endif