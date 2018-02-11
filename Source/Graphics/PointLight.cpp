#include "Graphics/PointLight.h"
#include "Graphics/GraphicsScene.h"

namespace Enjon {

	PointLight::PointLight()
	{
		mPosition 	= Vec3(0.0f, 0.0f, 0.0f);
		mColor 		= RGBA32_White(); 
		mAttenuationRate = 1.0f;
		mIntensity = 10.0f;
		mRadius = 100.0f;
	}

	PointLight::PointLight(Vec3& position, float attenuationRate, ColorRGBA32& color, float intensity, float radius)
	{
		mPosition = position;
		mAttenuationRate = attenuationRate;
		mColor = color;
		mIntensity = intensity;	
		mRadius = radius;
	}

	PointLight::~PointLight()
	{
		if (mGraphicsScene != nullptr)
		{
			mGraphicsScene->RemovePointLight(this);
		}
	}

	void PointLight::SetColor(ColorRGBA32& color)
	{
		mColor = color;
	}

	void PointLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;	
	}

	void PointLight::SetGraphicsScene(GraphicsScene* scene)
	{
		mGraphicsScene = scene;
	}

	void PointLight::SetPosition(Vec3& position)
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

	//==============================================================================================

	GraphicsScene* PointLight::GetGraphicsScene( ) const
	{
		return mGraphicsScene;
	}

	//============================================================================================== 
}