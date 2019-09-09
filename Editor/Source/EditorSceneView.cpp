// File: EditorSceneView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorSceneView.h"
#include "EditorAssetBrowserView.h"
#include "EditorApp.h"

#include <Engine.h>
#include <Graphics/GraphicsSubsystem.h>
#include <Entity/Components/StaticMeshComponent.h>
#include <Entity/Components/SkeletalMeshComponent.h>
#include <Entity/Components/SkeletalAnimationComponent.h>
#include <Scene/SceneManager.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>
#include <Graphics/FrameBuffer.h>
#include <Utils/FileUtils.h>

namespace Enjon
{
	//=================================================================

	EditorSceneView::EditorSceneView( EditorApp* app, Window* window )
		: EditorView( app, window, "Scene", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	} 

	//=================================================================

	void EditorSceneView::Initialize( )
	{
	}

	//=================================================================

	void EditorSceneView::UpdateView( )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );

		// Get context for main window
		GraphicsSubsystemContext* gfxCtx = gfx->GetMainWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
		u32 currentTextureId = gfxCtx->GetFrameBuffer( )->GetTexture( );
		//u32 currentTextureId = gfx->GetCurrentRenderTextureId( ); 

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		// Cache off cursor position for scene view
		Vec2 padding( 20.0f, 8.0f );
		//Vec2 padding( -20.0f, -40.0f );
		f32 width = ImGui::GetWindowWidth( ) - padding.x;
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		mSceneViewWindowPosition = Vec2( cursorPos.x, cursorPos.y );
		mSceneViewWindowSize = Vec2( width, height );

		ImTextureID img = ( ImTextureID )Int2VoidP(currentTextureId);
		ImGui::Image( img, ImVec2( width, height ),
			ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		// Update camera aspect ratio
		gfxCtx->GetGraphicsScene( )->GetActiveCamera( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );

		// Draw border around image
		ImDrawList* dl = ImGui::GetWindowDrawList( );
		ImVec2 a( mSceneViewWindowPosition.x, mSceneViewWindowPosition.y );
		ImVec2 b( mSceneViewWindowPosition.x + mSceneViewWindowSize.x, mSceneViewWindowPosition.y + mSceneViewWindowSize.y ); 
		dl->AddRect( a, b, ImColor( 0.0f, 0.64f, 1.0f, 0.48f ), 1.0f, 15, 1.5f ); 

		AssetHandle< Scene > mCurrentScene = EngineSubsystem( SceneManager )->GetScene( );
		if ( mCurrentScene )
		{
			String sceneLabel = "Scene: " + mCurrentScene->GetName( );
			ImVec2 sz = ImGui::CalcTextSize( sceneLabel.c_str( ) ); 
			ImVec2 rectPadding( 5.0f, 5.0f );
			ImVec2 padding( 10.0f, 10.0f ); 

			// Draw shadow text
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
			ImGui::SetCursorScreenPos( ImVec2( b.x - sz.x - padding.x + 1.0f, b.y - sz.y - padding.y + 1.0f ) ); 
			ImGui::Text( "%s", sceneLabel.c_str( ) );
			ImGui::PopStyleColor( );

			// Draw text
			ImGui::SetCursorScreenPos( ImVec2( b.x - sz.x - padding.x, b.y - sz.y - padding.y ) ); 
			ImGui::Text( "%s", sceneLabel.c_str( ) );
		} 

		// Render tool bar
		ImGui::SetCursorScreenPos( ImVec2( mSceneViewWindowPosition.x + mSceneViewWindowSize.x - 100.0f, mSceneViewWindowPosition.y + mSceneViewWindowSize.y * 0.01f ) );
		RenderToolBar( );

		WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );

		EditorAssetBrowserView* abv = mApp->GetEditorAssetBrowserView( );
		if ( abv->GetGrabbedAsset( ) && mWindow->IsMouseInWindow( ) && ws->NumberOfHoveredWindows() == 1 )
		{
			mWindow->SetFocus( );
		}

		if ( abv->GetGrabbedAsset( ) )
		{
			if ( mWindow->IsMouseInWindow( ) && mWindow->IsFocused( ) )
			{ 
				{
					String label = Utils::format( "Asset: %s", abv->GetGrabbedAsset( )->GetName().c_str() ).c_str( );
					ImVec2 txtSize = ImGui::CalcTextSize( label.c_str( ) );
					ImGui::SetNextWindowPos( ImVec2( ImGui::GetMousePos( ).x + 15.0f, ImGui::GetMousePos().y + 5.0f ) );
					ImGui::SetNextWindowSize( ImVec2( txtSize.x + 20.0f, txtSize.y ) );
					ImGui::Begin( "##grabbed_asset_window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
					{
						ImGui::Text( "%s", label.c_str( ) );
					}
					ImGui::End( ); 
				}

				if ( ImGui::IsMouseReleased( 0 ) && IsHovered( ) )
				{
					HandleAssetDrop( );
				} 
			} 
		}

		// Lose focus 
		if ( !IsFocused() )
		{
			// We just lost focus
			if ( mFocusSet )
			{
				mWindow->ShowMouseCursor( true ); 
			}

			mStartedFocusing = false;
			mFocusSet = false; 
		}
	}
 
	//=================================================================

	void EditorSceneView::RenderToolBar( )
	{
		// Just render the frame time for now
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );
		ImGui::SetCursorScreenPos( ImVec2( cursorPos.x + 1.0f, cursorPos.y + 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.2f, 0.2f, 0.2f, 1.0f ) );
		ImGui::Text( "Frame: %.3f", 1000.0f / ( f32 )ImGui::GetIO( ).Framerate );
		ImGui::SetCursorScreenPos( ImVec2( cursorPos.x + 1.0f + 73.0f, cursorPos.y + 1.0f ) );
		ImGui::Text( "ms" );
		ImGui::PopStyleColor( );
		ImGui::SetCursorScreenPos( cursorPos );
		ImGui::Text( "Frame: %.3f", 1000.0f / ( f32 )ImGui::GetIO( ).Framerate );
		ImGui::SetCursorScreenPos( ImVec2( cursorPos.x + 73.0f, cursorPos.y ) );
		ImGui::Text( "ms" );
	}
	
	//=================================================================

	void EditorSceneView::HandleAssetDrop( )
	{
		// Get projected mouse position
		Vec2 mp = GetSceneViewProjectedCursorPosition( );

		const Asset* grabbedAsset = mApp->GetEditorAssetBrowserView( )->GetGrabbedAsset( );

		// If material, then set material of overlapped object with this asset
		if ( grabbedAsset->Class( )->InstanceOf< Material >( ) )
		{
			// Check against graphics system for object
			GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 

			PickResult pickRes = gfx->GetPickedObjectResult( mp, gfx->GetMainWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( ) );

			u32 subMeshIdx = pickRes.mSubmeshIndex;

			Entity* ent = pickRes.mEntity.Get( );

			if ( ent )
			{
				// If static mesh component
				if ( ent->HasComponent< StaticMeshComponent >( ) )
				{
					StaticMeshComponent* smc = ent->GetComponent< StaticMeshComponent >( );
					smc->SetMaterial( grabbedAsset );
				}
				else if ( ent->HasComponent< SkeletalMeshComponent >( ) )
				{
					SkeletalMeshComponent* smc = ent->GetComponent< SkeletalMeshComponent >( );
					smc->SetMaterial( grabbedAsset );
				}
			}
		} 
		else if ( grabbedAsset->Class( )->InstanceOf< SkeletalMesh >( ) )
		{
			// Construct new entity in front of camera
			SkeletalMesh* mesh = grabbedAsset->ConstCast< SkeletalMesh >( );
			if ( mesh )
			{
				// Instantiate the archetype right in front of the camera for now
				GraphicsSubsystemContext* gfxCtx = GetWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
				Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
				Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
				EntityHandle handle = EngineSubsystem( EntityManager )->Allocate( );
				if ( handle )
				{
					Entity* newEnt = handle.Get( );
					SkeletalMeshComponent* smc = newEnt->AddComponent< SkeletalMeshComponent >( );
					SkeletalAnimationComponent* sac = newEnt->AddComponent< SkeletalAnimationComponent >( );
					smc->SetMesh( mesh );
					newEnt->SetLocalPosition( position );
					newEnt->SetName( mesh->GetName( ) );
				}
			}
		}
		else if ( grabbedAsset->Class( )->InstanceOf< Archetype >( ) )
		{
			Archetype* archType = grabbedAsset->ConstCast< Archetype >( );
			if ( archType )
			{
				// Instantiate the archetype right in front of the camera for now
				GraphicsSubsystemContext* gfxCtx = GetWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
				Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
				Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
				Vec3 scale = archType->GetRootEntity( ).Get( )->GetLocalScale( );
				EntityHandle handle = archType->Instantiate( Transform( position, Quaternion( ), scale ), GetWindow()->GetWorld() );
			}
		}
		else if ( grabbedAsset->Class( )->InstanceOf< SkeletalMesh >( ) )
		{
			// Do things...
		}
		else if ( grabbedAsset->Class( )->InstanceOf< Mesh >( ) )
		{
			// Construct new entity in front of camera
			Mesh* mesh = grabbedAsset->ConstCast< Mesh >( );
			if ( mesh )
			{
				// Instantiate the archetype right in front of the camera for now
				GraphicsSubsystemContext* gfxCtx = GetWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
				Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
				Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
				EntityHandle handle = EngineSubsystem( EntityManager )->Allocate( );
				if ( handle )
				{
					Entity* newEnt = handle.Get( );
					StaticMeshComponent* smc = newEnt->AddComponent< StaticMeshComponent >( );
					smc->SetMesh( mesh );
					newEnt->SetLocalPosition( position );
					newEnt->SetName( mesh->GetName( ) );
				}
			}
		}

	}
 
	//=================================================================

	void EditorSceneView::ProcessViewInput( )
	{ 
		// Not sure what to do in here, really... Need better abstractions for input controls, widgets, etc.
	}
 
	//=================================================================

	Vec2 EditorSceneView::GetSceneViewProjectedCursorPosition( )
	{
		// Need to get percentage of screen and things and stuff from mouse position
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 
		Input* input = EngineSubsystem( Input );
		iVec2 viewport = gfx->GetViewport( );
		Vec2 mp = input->GetMouseCoords( );
		
		// X screen percentage
		f32 pX = f32( mp.x - mSceneViewWindowPosition.x ) / f32( mSceneViewWindowSize.x );
		f32 pY = f32( mp.y - mSceneViewWindowPosition.y ) / f32( mSceneViewWindowSize.y ); 

		return Vec2( (s32)( pX * viewport.x ), (s32)( pY * viewport.y ) );
	}

	//==================================================================

	void EditorSceneView::CaptureState( )
	{
		Input* input = EngineSubsystem( Input );

		// Capture hovered state
		mIsHovered = ImGui::IsWindowHovered( ); 

		// Capture focused state
		mIsFocused = ( mIsHovered && ( input->IsKeyDown( KeyCode::RightMouseButton ) ) );
	}

	//==================================================================

	Vec2 EditorSceneView::GetCenterOfViewport( )
	{
		return Vec2( mSceneViewWindowPosition.x + mSceneViewWindowSize.x / 2.0f, mSceneViewWindowPosition.y + mSceneViewWindowSize.y / 2.0f );
	}

	//==================================================================

	void EditorSceneView::UpdateCamera( )
	{ 
		if ( IsFocused( ) && !mFocusSet )
		{
			if ( !mStartedFocusing )
			{
				mStartedFocusing = true;
				mFocusSet = true;
			}
		}

		Input* input = EngineSubsystem( Input ); 
 
		Enjon::Vec3 velDir( 0, 0, 0 ); 

		Window* window = this->GetWindow( );

		// Can't operate without a window
		assert( window != nullptr );

		World* world = window->GetWorld( );

		// Can't operate without a world
		assert( world != nullptr );

		GraphicsSubsystemContext* gsc = world->GetContext< GraphicsSubsystemContext >( );

		// Can't operate without a graphics subsystem context
		assert( gsc != nullptr );

		// Get viewport of window
		iVec2 viewPort = window->GetViewport( );

		if ( mStartedFocusing )
		{
			Vec2 mc = input->GetMouseCoords( );
			Vec2 center = GetCenterOfViewport( );
			mMouseCoordsDelta = Vec2( (f32)(viewPort.x) / 2.0f - mc.x, (f32)(viewPort.y) / 2.0f - mc.y ); 
			mStartedFocusing = false;
		}

		Camera* camera = gsc->GetGraphicsScene( )->GetActiveCamera( );

		// Set camera speed 
		Vec2 mw = input->GetMouseWheel( ).y;
		f32 mult = mw.y == 1.0f ? 1.5f : mw.y == -1.0f ? 0.75f : 1.0f;
		mCameraSpeed = Math::Clamp(mCameraSpeed * mult, 0.25f, 128.0f);

		if ( input->IsKeyDown( Enjon::KeyCode::W ) )
		{
			Enjon::Vec3 F = camera->Forward( );
			velDir += F;
		}
		if ( input->IsKeyDown( Enjon::KeyCode::S ) )
		{
			Enjon::Vec3 B = camera->Backward( );
			velDir += B;
		}
		if ( input->IsKeyDown( Enjon::KeyCode::A ) )
		{
			velDir += camera->Left( );
		}
		if ( input->IsKeyDown( Enjon::KeyCode::D ) )
		{
			velDir += camera->Right( );
		}

		// Normalize velocity
		velDir = Enjon::Vec3::Normalize( velDir );

		f32 avgDT = Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( );

		// Set camera position
		camera->SetPosition( camera->GetPosition() + ( mCameraSpeed * avgDT * velDir ) );

		// Set camera rotation
		// Get mouse input and change orientation of camera
		Enjon::Vec2 mouseCoords = input->GetMouseCoords( ); 


		// Set cursor to not visible
		window->ShowMouseCursor( false );

		// Reset the mouse coords after having gotten the mouse coordinates
		Vec2 center = GetCenterOfViewport( );
		//SDL_WarpMouseInWindow( window->GetWindowContext( ), (s32)center.x, (s32)center.y );
		SDL_WarpMouseInWindow( window->GetWindowContext( ), ( f32 )viewPort.x / 2.0f - mMouseCoordsDelta.x, ( f32 )viewPort.y / 2.0f - mMouseCoordsDelta.y );

		// Offset camera orientation
		f32 xOffset = Enjon::Math::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x - mMouseCoordsDelta.x ) * mMouseSensitivity / 100.0f;
		f32 yOffset = Enjon::Math::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y - mMouseCoordsDelta.y ) * mMouseSensitivity / 100.0f;
		camera->OffsetOrientation( xOffset, yOffset ); 
	}
}

