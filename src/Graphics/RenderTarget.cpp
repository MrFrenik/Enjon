#include "Graphics/RenderTarget.h"
#include "Utils/Errors.h"
#include <stdio.h>

namespace Enjon { namespace Graphics {

	RenderTarget::RenderTarget(uint32 _Width, uint32 _Height)
	{
		// Save extensions
		Width  = _Width;
		Height = _Height;

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Bind the diffuse render target
	glBindRenderbufferEXT(GL_RENDERBUFFER, diffuse);
	glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, Width, Height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse);

	// Bind the position render target
	glBindRenderbufferEXT(GL_RENDERBUFFER, normal);
	glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA32F_ARB, Width, Height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, normal);

	// Bind the normal render target
	glBindRenderbufferEXT(GL_RENDERBUFFER, albedo);
	glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA16F_ARB, Width, Height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, albedo);

    // - Position color buffer
    glGenTextures(1, &gDiffuse);
    glBindTexture(GL_TEXTURE_2D, gDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gDiffuse, 0);
    // - Normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // - Color + Specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    attachments[0] = GL_COLOR_ATTACHMENT0;
    attachments[1] = GL_COLOR_ATTACHMENT1;
    attachments[2] = GL_COLOR_ATTACHMENT2;
    glDrawBuffers(3, attachments);
    // - Create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // - Finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	RenderTarget::~RenderTarget()
	{
		glDeleteTextures(1, &Texture);
		glDeleteFramebuffersEXT(1, &FrameBufferID);
		glDeleteRenderbuffersEXT(1, &TargetID);
	}

	void RenderTarget::Bind()
	{
		// Bind our FBO and set the viewport to the proper size
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, Width, Height);

		// Clear the render targets
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);

		// Specify what to render an start acquiring
		// GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(3, attachments);
	}
			
	void RenderTarget::Unbind()
	{
		// Stop acquiring and unbind the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}}

