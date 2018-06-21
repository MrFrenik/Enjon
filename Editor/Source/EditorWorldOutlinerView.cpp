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

	bool IsParentInChildHierarchy( Entity* parent, Entity* child )
	{ 
		if ( !parent || !child || !child->GetParent() )
		{
			return false;
		} 

		if ( child->GetParent( ) == parent )
		{
			return true;
		} 

		return IsParentInChildHierarchy( parent, child->GetParent( ).Get( ) ); 
	}

	bool AttemptToAddChild( Entity* parent, Entity* child )
	{
		if ( !parent || !child )
		{
			return false;
		} 

		// If already has child, then return
		if ( parent->HasChild( child ) || child->HasChild( parent ) )
		{
			return false;
		}

		// If child has parent, then un-parent it
		if ( child->HasParent( ) )
		{
			child->RemoveParent( );
		}

		// Recursively determine if parent is in child's hiearchy already
		bool isInHierarchy = IsParentInChildHierarchy( parent, child ); 

		if ( !isInHierarchy )
		{
			parent->AddChild( child );
		}

		return true;
	}

	bool EditorWorldOutlinerView::DisplayEntityRecursively( const EntityHandle& handle, const u32& indentionLevel )
	{
		Entity* entity = handle.Get( );
		if ( entity == nullptr )
		{
			return false;
		} 

		EntityHandle selectedHandle = mApp->GetSelectedEntity( );
		Entity* selectedEnt = selectedHandle.Get( );
		Input* input = EngineSubsystem( Input );

		// Whether or not the entity is selected
		bool selected = entity == selectedEnt;
		// Whether or not the entity text is hovered
		bool hovered = false;

		// boolean to return
		bool anyItemHovered = false;

		static HashMap< u32, bool > mIsTreeOpen;

		const float indentionLevelOffset = 10.0f;
		const float boxIndentionLevelOffset = 20.0f;
		auto dl = ImGui::GetWindowDrawList( );

		ImVec2 a, b; 

		// Capture a
		a = ImVec2( ImGui::GetWindowPos().x, ImGui::GetCursorScreenPos( ).y );

		// Entity label text for name
		String entityLabelText = entity->GetName( );

		// Calculate label text size for bounding box
		ImVec2 textSize = ImGui::CalcTextSize( entityLabelText.c_str( ) ); 

		// Capture b
		b = ImVec2( a.x + ImGui::GetWindowSize().x, a.y + textSize.y );

		// Display entity name at indention level
		ImGui::SetCursorPosX( ImGui::GetCursorPosX() + indentionLevel * indentionLevelOffset ); 

		// Set hovered
		hovered = ImGui::IsMouseHoveringRect( a, b );

		// Add background if hovered or selected
		if ( hovered || selected )
		{
			ImColor hoveredColor = selected ? ImGui::GetColorU32( ImGuiCol_HeaderActive ) :  ImGui::GetColorU32( ImGuiCol_HeaderHovered );
			if ( !selected )
			{
				hoveredColor.Value.w = 0.4f; 
			}
			dl->AddRectFilled( a, b, hoveredColor ); 
		}

		// Draw triangle
		bool boxSelected = false;
		if ( entity->HasChildren( ) )
		{
			auto query = mIsTreeOpen.find( entity->GetID( ) );
			if ( query == mIsTreeOpen.end( ) )
			{
				mIsTreeOpen[ entity->GetID( ) ] = false;
			} 

			// Draw box for tree 
			float boxSize = 8.0f;
			ImVec2 ba = ImVec2( ImGui::GetCursorScreenPos( ).x + boxSize / 2.0f, ImGui::GetCursorScreenPos( ).y + textSize.y / 2.0f - boxSize / 2.0f );
			ImVec2 bb = ImVec2( ba.x + boxSize, ba.y + boxSize );
			dl->AddRect( ba, bb, ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) ); 

			if ( ImGui::IsMouseHoveringRect( ba, bb ) )
			{
				if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) ) 
				{
					boxSelected = true;
					auto query = mIsTreeOpen.find( entity->GetID( ) );
					if ( query == mIsTreeOpen.end( ) )
					{
						mIsTreeOpen[ entity->GetID( ) ] = true;
					}
					else
					{
						mIsTreeOpen[ entity->GetID( ) ] = !mIsTreeOpen[ entity->GetID( ) ]; 
					}	
				}
			}
		} 

		// Reset cursor position + box offset
		ImGui::SetCursorPosX( ImGui::GetCursorPosX() + boxIndentionLevelOffset ); 
		ImGui::SetCursorPosY( ImGui::GetCursorPosY( ) - textSize.y / 12.0f );

		// Bounding rect box hovering
		if ( hovered )
		{ 
			static bool held = false;

			// Select entity
			if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) && !boxSelected )
			{
				mApp->SelectEntity( entity );
			} 

			if ( ImGui::IsMouseDown( 0 ) && ImGui::IsMouseDragging( 0 ) )
			{
				if ( !held )
				{
					mGrabbedEntity = entity;
					held = true;
				}
			}

			if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) )
			{
				Entity* c = mGrabbedEntity.Get( );
				if ( c && entity != c && c->GetParent().Get() != entity )
				{
					bool added = AttemptToAddChild( entity, c ); 
					if ( added )
					{
						
						auto query = mIsTreeOpen.find( entity->GetID( ) );
						if ( query == mIsTreeOpen.end( ) )
						{
							mIsTreeOpen[ entity->GetID( ) ] = true;
						}
					}
				}

				held = false;
				mGrabbedEntity = EntityHandle::Invalid( );
			} 
		}

		// Display label text
		ImGui::Text( entityLabelText.c_str() ); 

		// Display all entity children
		if ( entity->HasChildren( ) )
		{
			if ( mIsTreeOpen[ entity->GetID( ) ] )
			{
				for ( auto& c : entity->GetChildren( ) )
				{
					anyItemHovered |= DisplayEntityRecursively( c, indentionLevel + 1 );
				} 
			}
		} 

		return ( anyItemHovered | hovered ); 
	}

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
		bool anyItemHovered = false;
		ImGui::ListBoxHeader( "##EntitiesListWorldOutliner", ImVec2( ImGui::GetWindowSize( ).x - padding.x, height ) );
		{
			EntityHandle selectedEntityHandle = mApp->GetSelectedEntity( );
			Entity* selectedEntity = selectedEntityHandle.Get( );
			for ( auto& e : entities->GetActiveEntities( ) )
			{ 
				// SKip over non-root entities
				if ( e->HasParent( ) )
				{
					continue;
				} 

				// Display entity
				anyItemHovered |= DisplayEntityRecursively( e ); 
			} 
		}
		ImGui::ListBoxFooter( ); 

		EditorWidgetManager* wm = mApp->GetEditorWidgetManager( );

		if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) )
		{
			if ( ImGui::IsMouseHoveringRect( ImGui::GetWindowPos( ), ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetWindowSize( ).x, ImGui::GetWindowPos( ).y + ImGui::GetWindowSize( ).y ) ) )
			{
				if ( !anyItemHovered )
				{
					if ( mGrabbedEntity )
					{
						mGrabbedEntity.Get( )->RemoveParent( );
						mGrabbedEntity = EntityHandle::Invalid( ); 
					} 
					else
					{
						mApp->DeselectEntity( );
					}
				} 
			}
		} 

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
