#include "Entity/Components/PointLightComponent.h"

namespace Enjon
{
	PointLightComponent::PointLightComponent()
	{
	}

	PointLightComponent::~PointLightComponent()
	{
	}

	void PointLightComponent::Update(float dt)
	{
	}

	void PointLightComponent::SetColor(EG::ColorRGBA16& color)
	{
		mLight.SetColor(color);
	}

	void PointLightComponent::SetIntensity(float intensity)
	{
		mLight.SetIntensity(intensity);
	}

	void PointLightComponent::SetPosition(EM::Vec3& position)
	{
		mLight.SetPosition(position);
	}

	void PointLightComponent::SetRadius(float radius)
	{
		mLight.SetRadius(radius);
	}

	void PointLightComponent::SetAttenuationRate(float rate)
	{
		mLight.SetAttenuationRate(rate);
	}

}