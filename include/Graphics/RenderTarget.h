#ifndef ENJON_RENDER_TARGET_H
#define ENJON_RENDER_TARGET_H
#pragma once

#include "System/Types.h"
#include "Math/Vec2.h"
#include "GLEW/glew.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { 


	class RenderTarget
	{
	 public:
			static enum class BindType
			{
				READ, 
				WRITE	
			};

			RenderTarget();
			RenderTarget(uint32 _Width, uint32 _Height, GLuint TexParam = GL_LINEAR);
			~RenderTarget();

			void Bind(BindType Type = BindType::WRITE);
			void Unbind();

			GLuint inline GetTexture() const 	{ return Texture; }
			GLuint inline GetDepth() const 		{ return DepthBuffer; }
			EM::Vec2 inline GetResolution() 	{ return EM::Vec2(Width, Height); }

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