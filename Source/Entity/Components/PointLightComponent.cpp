#include "Entity/Components/PointLightComponent.h"
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

	void PointLightComponent::ExplicitConstructor()
	{ 
		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	}

	//==================================================================================

	void PointLightComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		//if (mLight->GetGraphicsScene() != nullptr)
		//{
		//	GraphicsScene* gs = mLight->GetGraphicsScene( );
		//	gs->DeallocatePointLight( mLight );
		//}
	}

	//==================================================================================

	void PointLightComponent::PostConstruction( )
	{ 
		// Get graphics scene from world graphics context
		World* world = mEntity->GetWorld( )->ConstCast< World >( ); 
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 
		mLight = gs->AllocatePointLight( ); 
	}

	//==================================================================================

	void PointLightComponent::UpdateTransform( const Transform& transform )
	{
		mLight->SetPosition( transform.GetPosition( ) );
	}

	//==================================================================================

	void PointLightComponent::Update()
	{
		mLight->SetPosition( mEntity->GetWorldPosition( ) );
	} 

	//==================================================================================

	void PointLightComponent::SetColor( const ColorRGBA32& color )
	{
		mLight->SetColor( color );
	}

	//==================================================================================

	void PointLightComponent::SetIntensity( const f32& intensity )
	{
		mLight->SetIntensity( intensity );
	}

	//==================================================================================

	void PointLightComponent::SetPosition( const Vec3& position )
	{
		mLight->SetPosition( position );
	}

	//==================================================================================

	void PointLightComponent::SetRadius( const f32& radius )
	{
		mLight->SetRadius( radius );
	}

	//==================================================================================

	void PointLightComponent::SetAttenuationRate( const f32& rate )
	{
		mLight->SetAttenuationRate( rate );
	}

	//================================================================================== 

	Result PointLightComponent::OnEditorUI( )
	{
		// Inspect light 
		EngineSubsystem( ImGuiManager )->InspectObject( mLight.get_raw_ptr( ) );

		return Result::SUCCESS;
	} 

	//================================================================================== 
}