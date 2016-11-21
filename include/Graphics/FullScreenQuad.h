#ifndef ENJON_FULL_SCREEN_QUAD
#define ENJON_FULL_SCREEN_QUAD

#include "System/Types.h"
#include "GLEW/glew.h"

namespace Enjon { namespace Graphics { 

	struct FullScreenQuad
	{
			FullScreenQuad(uint32 _Width, uint32 _Height);
			~FullScreenQuad();

			void Bind();
			void Unbind();

			GLuint		FrameBufferID; 	// The FBO ID
			GLuint		TargetID; 		// The texture id
			u32			Texture; 		// The OpenGL texture for the diffuse render target
			u32			Width; 			// FBO width
			u32			Height; 		// FBO height
	};

}}

#endif