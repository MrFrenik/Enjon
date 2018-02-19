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

	Result PointLightComponent::SerializeData( ByteBuffer* buffer ) const
	{ 
		// TODO(): Would be nice to be able to serialize to another ObjectArchiver buffer and then concat. this buffer to that

		// Light position
		buffer->Write< f32 >( mLight.GetPosition( ).x );
		buffer->Write< f32 >( mLight.GetPosition( ).y );
		buffer->Write< f32 >( mLight.GetPosition( ).z ); 
		// Light intensity
		buffer->Write< f32 >( mLight.GetIntensity( ) );
		// Light attenuation rate
		buffer->Write< f32 >( mLight.GetAttenuationRate( ) );
		// Light radius
		buffer->Write< f32 >( mLight.GetRadius( ) );
		// Light Color
		buffer->Write< f32 >( mLight.GetColor( ).r );
		buffer->Write< f32 >( mLight.GetColor( ).g );
		buffer->Write< f32 >( mLight.GetColor( ).b );
		buffer->Write< f32 >( mLight.GetColor( ).a );

		return Result::SUCCESS;
	}

	//==================================================================================

	Result PointLightComponent::DeserializeData( ByteBuffer* buffer )
	{ 
		// Light position
		Vec3 position;
		position.x = buffer->Read< f32 >( );
		position.y = buffer->Read< f32 >( );
		position.z = buffer->Read< f32 >( );
		mLight.SetPosition( position );

		// Light intensity
		mLight.SetIntensity( buffer->Read< f32 >( ) );
		// Light attenuation rate
		mLight.SetAttenuationRate( buffer->Read< f32 >( ) );
		// Light radius
		mLight.SetRadius( buffer->Read< f32 >( ) );
		// Light Color
		ColorRGBA32 color;
		color.r = buffer->Read< f32 >( );
		color.g = buffer->Read< f32 >( );
		color.b = buffer->Read< f32 >( );
		color.a = buffer->Read< f32 >( );
		mLight.SetColor( color );

		return Result::SUCCESS;
	}

	//==================================================================================

}