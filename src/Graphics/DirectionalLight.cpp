#include "Graphics/DirectionalLight.h"
#include "Graphics/Scene.h"
#include "Math/Mat4.h"

namespace Enjon { namespace Graphics {

	DirectionalLight::DirectionalLight()
	{
		mDirection = EM::Vec3(1, 1, 1);
		mColor = EG::RGBA16_White();
		mIntensity = 1.0f;
	}

	DirectionalLight::DirectionalLight(EM::Vec3& direction, EG::ColorRGBA16& color, float intensity)
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

	void DirectionalLight::SetColor(EG::ColorRGBA16& color)
	{
		mColor = color;
	}

	void DirectionalLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;
	}

	void DirectionalLight::SetScene(EG::Scene* scene)
	{
		mScene = scene;
	}

}}


















