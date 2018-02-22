#include "Entity/Components/DirectionalLightComponent.h"
#include "Entity/EntityManager.h"
#include "Serialize/ByteBuffer.h"
#include "Graphics/GraphicsScene.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//==================================================================================

	void DirectionalLightComponent::ExplicitConstructor()
	{
		// Add renderable to scene
		GraphicsSubsystem* gs = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem >( )->ConstCast< GraphicsSubsystem >( );
		gs->GetGraphicsScene( )->AddDirectionalLight( &mLight );

		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	}

	//==================================================================================

	void DirectionalLightComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if (mLight.GetGraphicsScene() != nullptr)
		{
			mLight.GetGraphicsScene()->RemoveDirectionalLight(&mLight);
		}
	}

	//==================================================================================

	void DirectionalLightComponent::Update()
	{
		// Set direction to entity's forward vector
		mLight.SetDirection( mEntity->GetWorldRotation( ) * Vec3::ZAxis( ) );
	} 

	//==================================================================================

	void DirectionalLightComponent::SetColor(ColorRGBA32& color)
	{
		mLight.SetColor(color);
	}

	//==================================================================================

	void DirectionalLightComponent::SetIntensity(float intensity)
	{
		mLight.SetIntensity(intensity);
	} 

	//================================================================================== 
}
