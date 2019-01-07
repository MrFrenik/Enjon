#include "Entity/Components/DirectionalLightComponent.h"
#include "Entity/EntityManager.h"
#include "Serialize/ByteBuffer.h"
#include "Graphics/GraphicsScene.h"
#include "Graphics/GraphicsSubsystem.h"
#include "Base/World.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//==================================================================================

	void DirectionalLightComponent::ExplicitConstructor()
	{ 
		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	}

	//==================================================================================

	void DirectionalLightComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if (mLight->GetGraphicsScene() != nullptr)
		{
			mLight->GetGraphicsScene()->DeallocateDirectionalLight( mLight );
		}
	} 

	//==================================================================================

	void DirectionalLightComponent::PostConstruction( )
	{
		// Get graphics scene from world graphics context
		World* world = mEntity->GetWorld( )->ConstCast< World >( );
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 
		mLight = gs->AllocateDirectionalLight( ); 
	}

	//==================================================================================

	void DirectionalLightComponent::Update()
	{
		// Set direction to entity's forward vector
		mLight->SetDirection( mEntity->GetWorldRotation( ) * Vec3::ZAxis( ) );
	} 

	//==================================================================================

	void DirectionalLightComponent::UpdateTransform( const Transform& transform )
	{
		mLight->SetDirection( transform.GetEulerAngles( ).Normalize( ) );
	}

	//==================================================================================

	void DirectionalLightComponent::SetColor( const ColorRGBA32& color )
	{
		mLight->SetColor(color);
	}

	//==================================================================================

	void DirectionalLightComponent::SetIntensity( const f32& intensity )
	{
		mLight->SetIntensity(intensity);
	} 

	//================================================================================== 

	Result DirectionalLightComponent::OnEditorUI( )
	{
		// Inspect light 
		EngineSubsystem( ImGuiManager )->InspectObject( mLight.get_raw_ptr( ) );

		return Result::SUCCESS;
	}

	//================================================================================== 
}
