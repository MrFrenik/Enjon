#include "Graphics/GBuffer.h"

#include <vector>

namespace Enjon { namespace Graphics { 

	GBuffer::GBuffer()
	{
		glGenFramebuffersEXT(1, &FBO);
	}

	GBuffer::~GBuffer()
	{
		glDeleteFramebuffersEXT(1, &FBO);
	}

	bool GBuffer::Create(uint32 W, uint32 H)
	{
		if (Width == W && Height == H)
			return true;

		Width = W;
		Height = H;

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
	
		// Depth buffer	
		GLuint DepthRenderBuffer;

		glGenRenderbuffersEXT(1, &DepthRenderBuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, DepthRenderBuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, 
								 GL_DEPTH_COMPONENT, 
								 (GLsizei)Width, 
								 (GLsizei)Height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
									 GL_DEPTH_ATTACHMENT, 
									 GL_RENDERBUFFER_EXT, 
									 DepthRenderBuffer);

		std::vector<GLenum> DrawBuffers;

		auto AddRT = [&DrawBuffers, W, H](GLuint Tex, 
										  GLenum Attachment, 
										  GLint InternalFormat, 
										  GLenum Format, 
										  GLenum Type)
		{
			glBindTexture(GL_TEXTURE_2D, Tex);
			glTexImage2D(GL_TEXTURE_2D, 
						 0, 
						 InternalFormat, 
						 (GLsizei)W, 
						 (GLsizei)H, 
						 0,
						 Format,
						 GL_UNSIGNED_BYTE,
						 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, 
								    Attachment, 
								    Tex, 
								    0);

			DrawBuffers.push_back(Attachment);
		};

		AddRT(Diffuse, GL_COLOR_ATTACHMENT0_EXT, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
		AddRT(Specular, GL_COLOR_ATTACHMENT1_EXT, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
		AddRT(Normals, GL_COLOR_ATTACHMENT2_EXT, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
		AddRT(Depth, GL_DEPTH_ATTACHMENT_EXT, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);

		glDrawBuffers(DrawBuffers.size(), &DrawBuffers[0]);

		if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		return true;
	}

	void GBuffer::Bind()
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
	}

	void GBuffer::Unbind()
	{
		glFlush();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}


}}
