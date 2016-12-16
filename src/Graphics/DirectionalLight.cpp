#include "Graphics/DirectionalLight.h"

namespace Enjon { namespace Graphics {

	const int DirectionalLight::DirectionalLightShadowResolution = 1024;

	DirectionalLight::DirectionalLight()
	{
		Direction = EM::Vec3(1, 1, 1);
		Color = EG::RGBA16_White();
		Intensity = 1.0f;

		InitDepthMap();
	}

	DirectionalLight::DirectionalLight(EM::Vec3& _Direction, EG::ColorRGBA16& _Color, float _Intensity)
		: Direction(_Direction), Color(_Color), Intensity(_Intensity)
	{
		InitDepthMap();
	}

	DirectionalLight::~DirectionalLight()
	{

	}

	void DirectionalLight::InitDepthMap()
	{
		// Set up depth buffer for shadow
		glGenTextures(1, &DepthMap);
		glBindTexture(GL_TEXTURE_2D, DepthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DirectionalLightShadowResolution,
						DirectionalLightShadowResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}

	void DirectionalLight::BindDepth()
	{
		glViewport(0, 0, DirectionalLightShadowResolution, DirectionalLightShadowResolution);
		glBindFramebuffer(GL_FRAMEBUFFER, DepthMap);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void DirectionalLight::UnbindDepth()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPopAttrib();
	}
}}
