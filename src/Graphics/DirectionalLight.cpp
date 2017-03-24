#include "Graphics/DirectionalLight.h"
#include "Graphics/Scene.h"
#include "Math/Mat4.h"

namespace Enjon {

	DirectionalLight::DirectionalLight()
	{
		mDirection = EM::Vec3(1, 1, 1);
		mColor = RGBA16_White();
		mIntensity = 1.0f;
	}

	DirectionalLight::DirectionalLight(EM::Vec3& direction, ColorRGBA16& color, float intensity)
		: mDirection(direction), mColor(color), mIntensity(intensity)
	{
	}

	DirectionalLight::~DirectionalLight()
	{

	}

	void DirectionalLight::SetDirection(EM::Vec3& direction)
	{
		mDirection = direction;
	}

	void DirectionalLight::SetColor(ColorRGBA16& color)
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


















