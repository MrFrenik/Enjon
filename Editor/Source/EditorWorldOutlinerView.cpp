// File: EditorWorldOutlinerView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorWorldOutlinerView.h"
#include "EditorApp.h"

#include <Engine.h>
#include <Graphics/GraphicsSubsystem.h>
#include <Scene/SceneManager.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>

#include <fmt/format.h>

namespace Enjon
{
	//=================================================================

	EditorWorldOutlinerView::EditorWorldOutlinerView( EditorApp* app )
		: EditorView( app, "World Outliner", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	} 

	//=================================================================

	void EditorWorldOutlinerView::Initialize( )
	{
	}

	//=================================================================

	void EditorWorldOutlinerView::UpdateView( )
	{
		ImDrawList* dl = ImGui::GetWindowDrawList( );

		AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );

		if ( !scene )
		{
			return;
		}

		EntityManager* entities = EngineSubsystem( EntityManager );

		Input* input = EngineSubsystem( Input );

		// Print out scene name
		ImGui::Text( ( "Scene: " + scene->GetName( ) ).c_str( ) );

		ImGui::Separator( );

		// List out active entities
		Vec2 padding( 20.0f, 40.0f );
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		ImVec2 a, b;
		static bool held = false;
		static EntityHandle handle;
		ImGui::ListBoxHeader( "##EntitiesListWorldOutliner", ImVec2( ImGui::GetWindowSize( ).x - padding.x, height ) );
		{
			EntityHandle selectedEntityHandle = mApp->GetSelectedEntity( );
			Entity* selectedEntity = selectedEntityHandle.Get( );
			for ( auto& e : entities->GetActiveEntities( ) )
			{ 
				a = ImGui::GetCursorScreenPos( );
				if ( ImGui::Selectable( fmt::format( "{}##{}", e->GetName(), e->GetID() ).c_str( ), selectedEntity == e ) )
				{
					mApp->SelectEntity( e );
				} 
				b = ImVec2( a.x + ImGui::GetItemRectSize( ).x, a.y + ImGui::GetItemRectSize( ).y );

				if ( ImGui::IsMouseHoveringRect( a, b ) )
				{
					if ( input->IsKeyDown( KeyCode::LeftMouseButton ) )
					{
						if ( !held )
						{
							handle = e;
							std::cout << "Held\n";
							held = true;
						}
					}

					if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) )
					{
						Entity* p = handle.Get( );
						if ( p && e != p )
						{
							std::cout << "Parent: " << e->GetID() << ", Child: " << p->GetID() << "\n";
						}

						std::cout << "Released\n"; 
						held = false;
					} 
				}
			} 
		}
		ImGui::ListBoxFooter( ); 

		// Formatting
		ImVec2 csp = ImGui::GetCursorScreenPos( );
		ImVec2 la = ImVec2( csp.x, csp.y );
		ImVec2 lb = ImVec2( la.x + ImGui::GetWindowSize().x - padding.x, la.y );
		dl->AddLine( la, lb, ImGui::GetColorU32( ImGuiCol_Separator ) );

		ImGui::SetCursorScreenPos( ImVec2( csp.x, csp.y + 10.0f ) );

		// Display total amount of entities
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImColor( ImGui::GetColorU32( ImGuiCol_TextDisabled ) ) ) );
		ImGui::PushFont( EngineSubsystem( ImGuiManager )->GetFont( "Roboto-MediumItalic_14" ) );
		ImGui::Text( fmt::format( "{} Entities", entities->GetActiveEntities().size() ).c_str( ) );
		ImGui::PopFont( );
		ImGui::PopStyleColor( );

	}

	//=================================================================

	void EditorWorldOutlinerView::ProcessViewInput( )
	{ 
		// Not sure what to do in here, really... Need better abstractions for input controls, widgets, etc.
	} 

	//==================================================================

	void EditorWorldOutlinerView::CaptureState( )
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
}
