// File: EditorSceneView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorSceneView.h"
#include "EditorApp.h"

#include <Engine.h>
#include <Graphics/GraphicsSubsystem.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>

namespace Enjon
{
	//=================================================================

	EditorSceneView::EditorSceneView( EditorApp* app )
		: EditorView( app, "Scene", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
		// Don't like having to do this here...
		mApp->SetEditorSceneView( this );
	} 

	//=================================================================

	void EditorSceneView::Initialize( )
	{
	}

	//=================================================================

	void EditorSceneView::UpdateView( )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		u32 currentTextureId = gfx->GetCurrentRenderTextureId( ); 

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		// Cache off cursor position for scene view
		Vec2 padding( 20.0f, 8.0f );
		//Vec2 padding( -20.0f, -40.0f );
		f32 width = ImGui::GetWindowWidth( ) - padding.x;
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		mSceneViewWindowPosition = Vec2( cursorPos.x, cursorPos.y );
		mSceneViewWindowSize = Vec2( width, height );

		ImTextureID img = ( ImTextureID )currentTextureId;
		ImGui::Image( img, ImVec2( width, height ),
			ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		// Update camera aspect ratio
		gfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );

		// Draw border around image
		ImDrawList* dl = ImGui::GetWindowDrawList( );
		ImVec2 a( mSceneViewWindowPosition.x, mSceneViewWindowPosition.y );
		ImVec2 b( mSceneViewWindowPosition.x + mSceneViewWindowSize.x, mSceneViewWindowPosition.y + mSceneViewWindowSize.y ); 
		dl->AddRect( a, b, ImColor( 0.0f, 0.64f, 1.0f, 0.48f ), 1.0f, 15, 1.5f ); 

		//ImGui::SetCursorPosX( ImGui::GetWindowWidth( ) + padding.x - 20.0f );
		//ImGui::SetCursorScreenPos( ImVec2( cursorPos.x + ImGui::GetWindowWidth() - 60.0f, cursorPos.y + 10.0f ) );
		//ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.2f, 0.2f, 0.2f, 0.3f ) );
		//if ( ImGui::Button( "[ ]" ) )
		//{
		//	// Close all other tabs and make this the only one open
		//}
		//ImGui::PopStyleColor( );
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
		EditorWidgetManager* wm = mApp->GetEditorWidgetManager( );

		// Capture hovered state
		bool isHovered = ImGui::IsWindowHovered( ); 
		wm->SetHovered( this, isHovered );

		// Capture focused state
		bool isFocused = ( isHovered && ( input->IsKeyDown( KeyCode::RightMouseButton ) ) );
		wm->SetFocused( this, isFocused ); 
	}

	//==================================================================

	Vec2 EditorSceneView::GetCenterOfViewport( )
	{
		return Vec2( mSceneViewWindowPosition.x + mSceneViewWindowSize.x / 2.0f, mSceneViewWindowPosition.y + mSceneViewWindowSize.y / 2.0f );
	}

	//==================================================================
}

