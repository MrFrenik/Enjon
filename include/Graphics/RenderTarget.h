#ifndef ENJON_RENDER_TARGET_H
#define ENJON_RENDER_TARGET_H
#pragma once

#include "System/Types.h"
#include "GLEW/glew.h"

namespace Enjon { namespace Graphics { 

	struct RenderTarget
	{
			RenderTarget(uint32 _Width, uint32 _Height);
			~RenderTarget();

			void Bind();
			void Unbind();

			GLuint inline GetTexture() const 	{ return gNormal; }

			GLuint		FrameBufferID; 	// The FBO ID
			GLuint		TargetID; 		// The texture id
			GLuint		DepthBuffer; 	// Depth buffer handle
			u32			Texture; 		// The OpenGL texture for the diffuse render target
			u32			Width; 			// FBO width
			u32			Height; 		// FBO height

		    GLuint framebuffer;
		    GLuint textureColorbuffer;
		    GLuint rbo;

		    GLuint diffuse, normal, albedo;

			GLuint gBuffer;
		    GLuint gDiffuse, gNormal, gAlbedoSpec;
		    GLuint attachments[3];
		    GLuint rboDepth;
	};

}}



#endif