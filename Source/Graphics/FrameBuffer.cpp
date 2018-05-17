// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: FrameBuffer.cpp

#include "Graphics/FrameBuffer.h"
#include "Utils/Errors.h"
#include <stdio.h>

namespace Enjon 
{ 
	void FrameBuffer::ExplicitConstructor( )
	{
		mWidth = 1024;
		mHeight = 1024;
	}

	FrameBuffer::FrameBuffer(u32 width, u32 height, GLuint texParam)
	{
		// Save extensions
		mWidth  = width;
		mHeight = height;

		// Set up viewport
		mViewport = Vec4(0, 0, mWidth, mHeight);

	    glGenFramebuffers(1, &mFrameBufferID);
	    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);

	    // Bind the color render target
		glBindRenderbufferEXT(GL_RENDERBUFFER, mTargetID);
		glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, mWidth, mHeight);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mTargetID);

	    // - color buffer
	    glGenTextures(1, &mTexture);
	    glBindTexture(GL_TEXTURE_2D, mTexture);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, NULL);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParam);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParam);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
		glGenerateMipmap(GL_TEXTURE_2D);

		glGenTextures(1, &mDepthBuffer);
	    glBindTexture(GL_TEXTURE_2D, mDepthBuffer);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	   	GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0}; 
	   	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthBuffer, 0);
	    glDrawBuffer(GL_NONE);
	    glReadBuffer(GL_NONE);
	 
	    // - Create and attach depth buffer (renderbuffer)
	    // glGenRenderbuffers(1, &DepthBuffer);
	    // glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
	    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height);
	    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer);
	    // - Finally check if framebuffer is complete
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	        std::cout << "Framebuffer not complete!" << std::endl;

	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::ExplicitDestructor( )
	{
		glDeleteTextures(1, &mTexture);
		glDeleteFramebuffersEXT(1, &mFrameBufferID);
		glDeleteRenderbuffersEXT(1, &mTargetID);
		glDeleteRenderbuffersEXT(1, &mDepthBuffer);
	}

	void FrameBuffer::Bind(BindType type, bool clear)
	{
		switch( type )
		{
			case BindType::WRITE:
			{
				// Bind our FBO and set the viewport to the proper size
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferID);
				glPushAttrib(GL_VIEWPORT_BIT);
				glViewport(0, 0, mWidth, mHeight);

				if ( clear )
				{
					glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
				}

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glEnable(GL_TEXTURE_2D);

				// Specify what to render an start acquiring
				GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, buffers);
			} break;

			case BindType::READ:
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBufferID);
			} break;
		}
	}
			
	void FrameBuffer::Unbind()
	{
		// Stop acquiring and unbind the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPopAttrib();
	}
}

