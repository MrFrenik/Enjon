#include "Graphics/FrameBufferObject.h"

namespace Enjon { namespace Graphics { 

	/* Constructor */
	FrameBufferObject::FrameBufferObject(uint32 Width, uint32 Height)
	{
		// Save extensions
		m_width  = Width;
		m_height = Height;

		// Generate the OGL resources for what we need
		glGenFramebuffersEXT(1, &m_fbo);
		glGenRenderbuffersEXT(1, &m_diffuseRT);
		glGenRenderbuffersEXT(1, &m_positionRT);
		glGenRenderbuffersEXT(1, &m_normalsRT);
		glGenRenderbuffersEXT(1, &m_depthBuffer);

		// Bind the FBO so that the next operations will be bound to it
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

		// Bind the diffuse render target
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_diffuseRT);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, m_width, m_height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, m_diffuseRT);

		// Bind the position render target
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_positionRT);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA32F_ARB, m_width, m_height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER_EXT, m_positionRT);

		// Bind the normal render target
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_normalsRT);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA16F_ARB, m_width, m_height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_RENDERBUFFER_EXT, m_normalsRT);

		// Bind the depth buffer
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, m_width, m_height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);

		// Generate and bind the OGL texture for diffuse
		glGenTextures(1, &m_diffuseTexture);
		glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// Attach the texture to the FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_diffuseTexture, 0);

		// Generate and bind the OGL texture for positions
		glGenTextures(1, &m_positionTexture);
		glBindTexture(GL_TEXTURE_2D, m_positionTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// Attach the texture to the FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_positionTexture, 0);

		// Generate and bind the OGL texture for normals
		glGenTextures(1, &m_normalsTexture);
		glBindTexture(GL_TEXTURE_2D, m_normalsTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// Attach the texture to the FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, m_normalsTexture, 0);

		// Check if all worked fine and unbind the FBO
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if( status != GL_FRAMEBUFFER_COMPLETE_EXT)
			throw new std::exception("Can't initialize an FBO render texture. FBO initialization failed.");

		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	}

	/* Destructor */
	FrameBufferObject::~FrameBufferObject()
	{
		glDeleteTextures(1, &m_normalsTexture);
		glDeleteTextures(1, &m_positionTexture);
		glDeleteTextures(1, &m_diffuseTexture);
		glDeleteFramebuffersEXT(1, &m_fbo);
		glDeleteRenderbuffersEXT(1, &m_diffuseRT);
		glDeleteRenderbuffersEXT(1, &m_positionRT);
		glDeleteRenderbuffersEXT(1, &m_normalsRT);
		glDeleteRenderbuffersEXT(1, &m_depthBuffer);
	}

	void FrameBufferObject::Bind()
	{
		// Bind our FBO and set the viewport to the proper size
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0,0,m_width, m_height);

		// Clear the render targets
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		// glClearColor(0.3f, 0.3f, 0.3f, 1.0f );

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		
		// glEnable(GL_DEPTH_TEST);
		// glDepthMask(true);

		// Specify what to render an start acquiring
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
		glDrawBuffers(3, buffers);
	}

	void FrameBufferObject::Unbind()
	{
		// Stop acquiring and unbind the FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glPopAttrib();
	}

}}

