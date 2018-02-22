#include "Entity/Components/PointLightComponent.h"
#include "Entity/EntityManager.h"
#include "Serialize/ByteBuffer.h"
#include "Graphics/GraphicsScene.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//==================================================================================

	void PointLightComponent::ExplicitConstructor()
	{
		// Add renderable to scene
		GraphicsSubsystem* gs = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem >( )->ConstCast< GraphicsSubsystem >( );
		gs->GetGraphicsScene( )->AddPointLight( &mLight );

		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	}

	//==================================================================================

	void PointLightComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if (mLight.GetGraphicsScene() != nullptr)
		{
			mLight.GetGraphicsScene()->RemovePointLight(&mLight);
		}
	}

	//==================================================================================

	void PointLightComponent::Update()
	{
		mLight.SetPosition(mEntity->GetWorldPosition());	
	} 

	//==================================================================================

	void PointLightComponent::SetColor(ColorRGBA32& color)
	{
		mLight.SetColor(color);
	}

	//==================================================================================

	void PointLightComponent::SetIntensity(float intensity)
	{
		mLight.SetIntensity(intensity);
	}

	//==================================================================================

	void PointLightComponent::SetPosition(Vec3& position)
	{
		mLight.SetPosition(position);
	}

	//==================================================================================

	void PointLightComponent::SetRadius(float radius)
	{
		mLight.SetRadius(radius);
	}

	//==================================================================================

	void PointLightComponent::SetAttenuationRate(float rate)
	{
		mLight.SetAttenuationRate(rate);
	}

	//================================================================================== 
}