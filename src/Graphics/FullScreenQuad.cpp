#include "Graphics/FullScreenQuad.h"

namespace Enjon { namespace Graphics { 

	FullScreenQuad::FullScreenQuad(uint32 _Width, uint32 _Height)
	{
		static const GLfloat VertexBufferData[] = 
		{
			-1.0f, -1.0f, 0.0f, 
			1.0f, -1.0f, 0.0f, 
			-1.0f, -1.0f, 0.0f, 
			-1.0f, 1.0f, 0.0f, 
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};
	}

	FullScreenQuad::~FullScreenQuad()
	{

	}

	void FullScreenQuad::Bind()
	{

	}

	void FullScreenQuad::Unbind()
	{

	}
}}
