#include "Entity/Components/PointLightComponent.h"
#include "Entity/EntityManager.h"

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
		mLight.SetPosition(mEntity->GetWorldPosition());	
	} 

	void PointLightComponent::SetColor(ColorRGBA16& color)
	{
		mLight.SetColor(color);
	}

	void PointLightComponent::SetIntensity(float intensity)
	{
		mLight.SetIntensity(intensity);
	}

	void PointLightComponent::SetPosition(Vec3& position)
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