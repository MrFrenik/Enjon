#include "Graphics/SpotLight.h"
#include "Graphics/Scene.h"


namespace Enjon {

	SpotLight::SpotLight()
	{
		mPosition 	= EM::Vec3(0.0f, 0.0f, 0.0f);
		mColor 		= RGBA16_White(); 
		mParams 	= SpotLightParameters(0.0f, 0.0f, 0.0f, EM::Vec3(0, 0, 0), 0.0f, 0.0f);
	}

	SpotLight::SpotLight(EM::Vec3& position, SLParams& params, ColorRGBA16& color, float intensity)
	{
		mPosition 	= position;
		mParams 	= params;
		mColor 		= color;
		mIntensity 	= intensity;

		const float constant 	= mParams.mConstant;
		const float linear 		= mParams.mLinear;
		const float quadratic 	= mParams.mQuadratic;

		// Not really doing anything with these for some reason...
	}

	SpotLight::~SpotLight()
	{
	}

	void SpotLight::SetColor(ColorRGBA16& color)
	{
		mColor = color;
	}

	void SpotLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;
	}

	void SpotLight::SetScene(Scene* scene)
	{
		mScene = scene;	
	}
	void SpotLight::SetPosition(EM::Vec3& position)
	{
		mPosition = position;
	}

	void SpotLight::SetDirection(EM::Vec3& direction)
	{
		mParams.mDirection = direction;
	}

	void SpotLight::SetParams(SLParams& params)
	{
		mParams = params;
	}

}