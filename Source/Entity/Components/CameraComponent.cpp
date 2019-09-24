#include "Entity/Components/CameraComponent.h"
#include "Entity/EntityManager.h" 
#include "Graphics/GraphicsScene.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//====================================================================

	void CameraComponent::ExplicitConstructor()
	{ 
		// Get graphics subsytem
		GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );

		// Set up camera using viewport from graphics subsystem
		mCamera = Camera( gs->GetViewport( ) );

		// Add renderable to scene
		gs->GetGraphicsScene( )->AddCamera( &mCamera );

		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	} 

	//====================================================================

	void CameraComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if ( mCamera.GetGraphicsScene() != nullptr )
		{
			mCamera.GetGraphicsScene( )->RemoveCamera( &mCamera );
		}
	} 

	//==================================================================== 

	void CameraComponent::Initialize( )
	{
		// Set camera to active camera in scene
		if ( mCamera.GetGraphicsScene( ) )
		{
			mCamera.GetGraphicsScene( )->SetActiveCamera( &mCamera );
		}
	}

	//==================================================================== 

	void CameraComponent::SetActiveCamera( )
	{
		if ( mCamera.GetGraphicsScene( ) )
		{
			mCamera.GetGraphicsScene( )->SetActiveCamera( &mCamera );
		}
	}

	//==================================================================== 

	void CameraComponent::Update( )
	{
		Transform wt = mEntity->GetWorldTransform( );
		mCamera.SetPosition( wt.GetPosition( ) ); 
		mCamera.SetRotation( wt.GetRotation().Normalize() );
	}
	
	//==================================================================== 

	GraphicsScene* CameraComponent::GetGraphicsScene() const
	{ 
		return mCamera.GetGraphicsScene(); 
	} 

	//====================================================================

	void CameraComponent::SetGraphicsScene(GraphicsScene* scene)
	{
		mCamera.SetGraphicsScene(scene);
	} 

	//==================================================================== 

	Camera* CameraComponent::GetCamera( )
	{
		return &mCamera;
	}

	//==================================================================== 

	Result CameraComponent::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		igm->DebugDumpObject( &mCamera );
		return Result::SUCCESS;
	}

	//==================================================================== 

	Result CameraComponent::OnViewportDebugDraw()
	{
		// Draw lines extending as frustum from center point of camera
		// I guess this is right?...I dunno...
		Entity* ent = GetEntity();
		Transform wt = ent->GetWorldTransform();
		Vec3 wp = wt.GetPosition();
		Vec3 right = ent->Right();
		Vec3 up = ent->Up();
		Vec3 bward = -ent->Forward();
		Quaternion negAAR = Quaternion::AngleAxis( Math::ToRadians( -45.f ), right );
		Quaternion negAAU = Quaternion::AngleAxis( Math::ToRadians( -45.f ), up );
		Quaternion posAAR = Quaternion::AngleAxis( Math::ToRadians( 45.f ), right );
		Quaternion posAAU = Quaternion::AngleAxis( Math::ToRadians( 45.f ), up );

		//Vec3 tl = wp + Vec3::Normalize(-bward * Quaternion::AngleAxis(Math::ToRadians(45.f), ent->Right()) * Quaternion::AngleAxis(Math::ToRadians(-45.f), ent->Up()) * 5.f );
		//Vec3 tr = wp + Vec3::Normalize(-bward * Quaternion::AngleAxis(Math::ToRadians(45.f), ent->Right()) * Quaternion::AngleAxis(Math::ToRadians(45.f), ent->Up()) * 5.f );
		//Vec3 bl = wp + Vec3::Normalize(-bward * Quaternion::AngleAxis(Math::ToRadians(-45.f), ent->Right()) * Quaternion::AngleAxis(Math::ToRadians(-45.f), ent->Up()) * 5.f );
		//Vec3 br = wp + Vec3::Normalize(-bward * Quaternion::AngleAxis(Math::ToRadians(45.f), ent->Right()) * Quaternion::AngleAxis(Math::ToRadians(45.f), ent->Up()) * 5.f );

		Vec3 tl = wp + Vec3::Normalize( bward * posAAR * negAAU ) * 5.f;
		Vec3 tr = wp + Vec3::Normalize( bward * posAAR * posAAU ) * 5.f;
		Vec3 bl = wp + Vec3::Normalize( bward * negAAR * negAAU ) * 5.f;
		Vec3 br = wp + Vec3::Normalize( bward * negAAR * posAAU ) * 5.f;

		// This should be a context thing...
		//EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wt.GetPosition(), tl );
		//EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wt.GetPosition(), tr );
		//EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wt.GetPosition(), bl );
		//EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wt.GetPosition(), br ); 

		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wp, tl);
		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wp, tr);
		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wp, bl);
		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( wp, br);

		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( tl, tr );
		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( tr, br );
		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( br, bl );
		EngineSubsystem( GraphicsSubsystem )->DrawDebugLine( bl, tl ); 

		return Result::SUCCESS;
	}

	//==================================================================== 
}








