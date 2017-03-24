#include "Graphics/PointLight.h"
#include "Graphics/Scene.h"

namespace Enjon {

	PointLight::PointLight()
	{
		mPosition 	= EM::Vec3(0.0f, 0.0f, 0.0f);
		mColor 		= RGBA16_White(); 
		mAttenuationRate = 1.0f;
		mIntensity = 10.0f;
		mRadius = 100.0f;
	}

	PointLight::PointLight(EM::Vec3& position, float attenuationRate, ColorRGBA16& color, float intensity, float radius)
	{
		mPosition = position;
		mAttenuationRate = attenuationRate;
		mColor = color;
		mIntensity = intensity;	
		mRadius = radius;
	}

	PointLight::~PointLight()
	{
		if (mScene != nullptr)
		{
			mScene->RemovePointLight(this);
		}
	}

	void PointLight::SetColor(ColorRGBA16& color)
	{
		mColor = color;
	}

	void PointLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;	
	}

	void PointLight::SetScene(Scene* scene)
	{
		mScene = scene;
	}

	void PointLight::SetPosition(EM::Vec3& position)
	{
		mPosition = position;
	}

	void PointLight::SetAttenuationRate(float rate)
	{
		mAttenuationRate = rate;
	}

	void PointLight::SetRadius(float radius)
	{
		mRadius = radius;
	}
}