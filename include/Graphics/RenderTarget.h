#ifndef ENJON_RENDER_TARGET_H
#define ENJON_RENDER_TARGET_H
#pragma once

#include "System/Types.h"
#include "GLEW/glew.h"

namespace Enjon { namespace Graphics { 

	class RenderTarget
	{
		public:
			RenderTarget::RenderTarget(uint32 _Width, uint32 _Height);
			~RenderTarget();

			void Bind();
			void Unbind();

			GLuint inline GetTexture() const 	{ return Texture; }

		private:
			GLuint		FrameBufferID; 	// The FBO ID
			GLuint		TargetID; 		// The texture id
			GLuint		DepthBuffer; 	// Depth buffer handle
			GLuint		Texture; 		// The OpenGL texture for the diffuse render target
			u32			Width; 			// FBO width
			u32			Height; 		// FBO height
	};

}}



#endif