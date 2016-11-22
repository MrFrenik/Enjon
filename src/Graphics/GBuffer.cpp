#include "Graphics/GBuffer.h"
#include "Utils/Errors.h"
#include "Defines.h"
#include <stdio.h>

namespace Enjon { namespace Graphics {

	GBuffer::GBuffer(uint32 _Width, uint32 _Height)
	{
		// Save extensions
		Width  = _Width;
		Height = _Height;
		Viewport = EM::Vec4(0, 0, Width, Height);

	    glGenFramebuffers(1, &FBO);
	    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	    // Bind the diffuse render target
		glBindRenderbufferEXT(GL_RENDERBUFFER, TargetIDs[(u32)GBufferTextureType::DIFFUSE]);
		glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, Width, Height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, TargetIDs[(u32)GBufferTextureType::DIFFUSE]);

	    // Bind the normal render target
		glBindRenderbufferEXT(GL_RENDERBUFFER, TargetIDs[(u32)GBufferTextureType::NORMAL]);
		glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, Width, Height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, TargetIDs[(u32)GBufferTextureType::NORMAL]);

	    // Bind the emissive render target
		glBindRenderbufferEXT(GL_RENDERBUFFER, TargetIDs[(u32)GBufferTextureType::NORMAL]);
		glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, Width, Height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, TargetIDs[(u32)GBufferTextureType::EMISSIVE]);

	    // - Diffuse buffer
	    glGenTextures(1, &Textures[(u32)GBufferTextureType::DIFFUSE]);
	    glBindTexture(GL_TEXTURE_2D, Textures[(u32)GBufferTextureType::DIFFUSE]);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Textures[(u32)GBufferTextureType::DIFFUSE], 0);

	    // - Normal buffer
	    glGenTextures(1, &Textures[(u32)GBufferTextureType::NORMAL]);
	    glBindTexture(GL_TEXTURE_2D, Textures[(u32)GBufferTextureType::NORMAL]);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Textures[(u32)GBufferTextureType::NORMAL], 0);

	    // - Emissive buffer
	    glGenTextures(1, &Textures[(u32)GBufferTextureType::EMISSIVE]);
	    glBindTexture(GL_TEXTURE_2D, Textures[(u32)GBufferTextureType::EMISSIVE]);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Textures[(u32)GBufferTextureType::EMISSIVE], 0);
	 
	    // - Create and attach depth buffer (renderbuffer)
	    glGenRenderbuffers(1, &Textures[(u32)GBufferTextureType::DEPTH]);
	    glBindRenderbuffer(GL_RENDERBUFFER, Textures[(u32)GBufferTextureType::DEPTH]);
	    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height);
	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Textures[(u32)GBufferTextureType::DEPTH]);

	    // - Finally check if framebuffer is complete
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) EU::FatalError("GBuffer::Constructor::Gbuffer could not be created.");

	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GBuffer::SetViewport(const EM::Vec4& _Viewport)
	{
		Viewport = _Viewport;	
	}

	GBuffer::~GBuffer()
	{

	}

	void GBuffer::Bind()
	{
		// Bind our FBO and set the viewport to the proper size
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport((u32)Viewport.x, (u32)Viewport.y, (u32)Viewport.z, (u32)Viewport.w);

		// Clear the render targets
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);

		// Specify what to render an start acquiring
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, buffers);
	}
			
	void GBuffer::Unbind()
	{
		// Stop acquiring and unbind the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPopAttrib();
	}
}}

