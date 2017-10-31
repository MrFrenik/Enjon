#include "Graphics/SpotLight.h"
#include "Graphics/Scene.h"


namespace Enjon {

	SpotLight::SpotLight()
	{
		mPosition 	= Vec3(0.0f, 0.0f, 0.0f);
		mColor 		= RGBA32_White(); 
		mParams 	= SpotLightParameters(0.0f, 0.0f, 0.0f, Vec3(0, 0, 0), 0.0f, 0.0f);
	}

	SpotLight::SpotLight(Vec3& position, SLParams& params, ColorRGBA32& color, float intensity)
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

	void SpotLight::SetColor(ColorRGBA32& color)
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
	void SpotLight::SetPosition(Vec3& position)
	{
		mPosition = position;
	}

	void SpotLight::SetDirection(Vec3& direction)
	{
		mParams.mDirection = direction;
	}

	void SpotLight::SetParams(SLParams& params)
	{
		mParams = params;
	}

}