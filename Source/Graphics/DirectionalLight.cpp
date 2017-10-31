#include "Graphics/DirectionalLight.h"
#include "Graphics/Scene.h"
#include "Math/Mat4.h"

namespace Enjon {

	DirectionalLight::DirectionalLight()
	{
		mDirection = Vec3(1, 1, 1);
		mColor = RGBA32_White();
		mIntensity = 1.0f;
	}

	DirectionalLight::DirectionalLight(Vec3& direction, ColorRGBA32& color, float intensity)
		: mDirection(direction), mColor(color), mIntensity(intensity)
	{
	}

	DirectionalLight::~DirectionalLight()
	{

	}

	void DirectionalLight::SetDirection(Vec3& direction)
	{
		mDirection = direction;
	}

	void DirectionalLight::SetColor(ColorRGBA32& color)
	{
		mColor = color;
	}

	void DirectionalLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;
	}

	void DirectionalLight::SetScene(Scene* scene)
	{
		mScene = scene;
	}

}


















