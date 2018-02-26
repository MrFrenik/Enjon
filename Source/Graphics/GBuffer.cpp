#include "Graphics/GBuffer.h"
#include "Utils/Errors.h"
#include "Defines.h"
#include <stdio.h>

namespace Enjon {

#define CREATE_RENDER_TARGET(InternalFormat, Format, DataType, GBufferAttachment)\
	{\
		u32 index = (u32)GBufferAttachment;\
		glBindRenderbufferEXT(GL_RENDERBUFFER, TargetIDs[index]);\
		glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, Width, Height);\
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, TargetIDs[index]);\
		\
		glGenTextures(1, &Textures[index]);\
		glBindTexture(GL_TEXTURE_2D, Textures[index]);\
		\
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, DataType, NULL);\
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);\
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );\
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );\
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );\
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, Textures[ index ], 0 );\
		\
		glGenerateMipmap( GL_TEXTURE_2D );\
		\
		glBindTexture( GL_TEXTURE_2D, 0 );\
	}

	GBuffer::GBuffer(u32 _Width, u32 _Height)
	{
		// Save extensions
		Width  = _Width;
		Height = _Height;
		Viewport = Vec4(0, 0, Width, Height);

	    glGenFramebuffers(1, &FBO);
	    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		
		CREATE_RENDER_TARGET( GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, GBufferTextureType::ALBEDO )
		CREATE_RENDER_TARGET( GL_RGB16F, GL_RGB, GL_FLOAT, GBufferTextureType::NORMAL )
		CREATE_RENDER_TARGET( GL_RGB16F, GL_RGB, GL_FLOAT, GBufferTextureType::EMISSIVE )
		CREATE_RENDER_TARGET( GL_RGB16F, GL_RGB, GL_FLOAT, GBufferTextureType::MAT_PROPS )
		CREATE_RENDER_TARGET( GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, GBufferTextureType::OBJECT_ID )
		CREATE_RENDER_TARGET( GL_RG16F, GL_RG, GL_FLOAT, GBufferTextureType::VELOCITY )

		// Bind depth render buffer
		glBindRenderbufferEXT( GL_RENDERBUFFER, DepthBuffer );
		glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_RGBA, Width, Height );
		glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer );

		// - Depth buffer texture
		glGenTextures( 1, &DepthTexture );
		glBindTexture( GL_TEXTURE_2D, DepthTexture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0 ); 

		glBindTexture( GL_TEXTURE_2D, 0 );
		
	    // - Finally check if framebuffer is complete
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) EU::FatalError("GBuffer::Constructor::Gbuffer could not be created.");

	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GBuffer::SetViewport(const Vec4& _Viewport)
	{
		Viewport = _Viewport;	
	}

	GBuffer::~GBuffer()
	{
		for (u32 i = 0; i < (u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT; ++i)
		{
			glDeleteTextures(1, &Textures[i]);
			glDeleteRenderbuffers(1, &TargetIDs[i]);
		}

		// Clean up buffers
		glDeleteFramebuffers(1, &FBO);
		glDeleteRenderbuffers(1, &DepthBuffer);
	}

	void GBuffer::Bind(BindType Type, bool clear)
	{
		switch(Type)
		{
			case BindType::WRITE:
			{
				// Bind our FBO and set the viewport to the proper size
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
				glPushAttrib(GL_VIEWPORT_BIT);
				glViewport((u32)Viewport.x, (u32)Viewport.y, (u32)Viewport.z, (u32)Viewport.w);

				// Clear the render targets
				if ( clear )
				{
					glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
				}

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_DEPTH);

				// Specify what to render an start acquiring
				GLenum buffers[(u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT];
				for (auto i = 0; i < (u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT; i++)
				{
					buffers[i] = GL_COLOR_ATTACHMENT0 + i;
				}

				glDrawBuffers((u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT, buffers);
			} break;

			case BindType::READ:
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
			} break;
		}
	}
			
	void GBuffer::Unbind()
	{
		// Stop acquiring and unbind the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPopAttrib();
	}

	const char* GBuffer::FrameBufferToString(u32 i)
	{
		switch (i)
		{
			case 0: return "Albedo";
			case 1: return "Normal";
			case 2: return "Emissive";
			case 3: return "Materials";
			case 4: return "ObjectID";
			case 5: return "Velocity";
			default: return "Unknown";
		}	
	}
}

