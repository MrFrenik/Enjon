#include "Graphics/DirectionalLight.h"
#include "Math/Mat4.h"

namespace Enjon { namespace Graphics {

	const int DirectionalLight::DirectionalLightShadowResolution = 1024;
	const float DirectionalLight::DirectionalLightNear = 1.0f;
	const float DirectionalLight::DirectionalLightFar = 7.5f;

	DirectionalLight::DirectionalLight()
	{
		Position = EM::Vec3(1, 1, 1);
		Color = EG::RGBA16_White();
		Intensity = 1.0f;
	}

	DirectionalLight::DirectionalLight(EM::Vec3& _Position, EG::ColorRGBA16& _Color, float _Intensity)
		: Position(_Position), Color(_Color), Intensity(_Intensity)
	{
		InitDepthMap();
	}

	DirectionalLight::~DirectionalLight()
	{

	}

	void DirectionalLight::InitDepthMap()
	{
		DepthTarget = EG::RenderTarget(DirectionalLightShadowResolution, DirectionalLightShadowResolution);

		/*
	    glGenFramebuffers(1, &DepthFBO);
	    glBindFramebuffer(GL_FRAMEBUFFER, DepthFBO);
	 
	    // - Create and attach depth buffer (renderbuffer)
	    glGenRenderbuffers(1, &DepthBuffer);
	    glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
	    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, DirectionalLightShadowResolution, DirectionalLightShadowResolution);
	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer);
	    // - Finally check if framebuffer is complete
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	        std::cout << "Framebuffer not complete!" << std::endl;

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    */
	}

	void DirectionalLight::BindDepth()
	{
		/*
		// Bind our FBO and set the viewport to the proper size
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, DepthFBO);
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, DirectionalLightShadowResolution, DirectionalLightShadowResolution);

		// Clear the render targets
		glClear( GL_DEPTH_BUFFER_BIT );

		// glActiveTextureARB(GL_TEXTURE0_ARB);
		// glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH);

		// Specify what to render an start acquiring
		// GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		// glDrawBuffers(1, buffers);
		*/
		DepthTarget.Bind();
	}

	void DirectionalLight::UnbindDepth()
	{
		DepthTarget.Unbind();
		// Stop acquiring and unbind the FBO
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// glPopAttrib();
	}
}}
