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

	void AttemptToAddChild( Entity* parent, Entity* child )
	{
		if ( !parent || !child )
		{
			return;
		} 

		// If already has child, then return
		if ( parent->HasChild( child ) || child->HasChild( parent ) )
		{
			return;
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
	}

	/*
	bool EditorWorldOutlinerView::DisplayEntityRecursively( const EntityHandle& entity )
	{
		Entity* e = entity.Get( );
		if ( !e ) 
		{
			return false;
		}

		Input* input = EngineSubsystem( Input );
		EntityHandle selectedEntityHandle = mApp->GetSelectedEntity( );
		Entity* selectedEntity = selectedEntityHandle.Get( );
		ImVec2 a, b;
		static bool held = false;
		bool anyItemHovered = false;
		static ImVec2 rectSize;
 
		// Capture a
		a = ImGui::GetCursorScreenPos( ); 
		b = ImVec2( a.x + rectSize.x, a.y + rectSize.y );

		// Draw background rect
		if ( ImGui::IsMouseHoveringRect( a, b ) )
		{
			ImGui::GetWindowDrawList( )->AddRectFilled( a, b, ImGui::GetColorU32( ImGuiCol_HeaderHovered ) );
		}

		if ( e->HasChildren( ) )
		{ 
			if ( ImGui::TreeNodeEx( fmt::format( "{}##{}", e->GetName( ), e->GetID( ) ).c_str( ), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				rectSize = ImVec2( ImGui::GetWindowWidth(), ImGui::GetItemRectSize( ).y );
				b = ImVec2( a.x + ImGui::GetItemRectSize( ).x, a.y + ImGui::GetItemRectSize( ).y );

				// Display children if collapsed
				for ( auto& c : e->GetChildren( ) )
				{
					anyItemHovered |= DisplayEntityRecursively( c );
				} 
				ImGui::TreePop( );
			}
			else
			{
				b = ImVec2( a.x + ImGui::GetItemRectSize( ).x, a.y + ImGui::GetItemRectSize( ).y ); 
			} 
		}
		else
		{
			if ( ImGui::Selectable( fmt::format( "{}##{}", e->GetName( ), e->GetID( ) ).c_str( ), selectedEntity == e ) )
			{
				mApp->SelectEntity( e );
			} 
			b = ImVec2( a.x + ImGui::GetItemRectSize( ).x, a.y + ImGui::GetItemRectSize( ).y ); 
		}

		if ( ImGui::IsMouseHoveringRect( a, b ) )
		{ 
			anyItemHovered = true;

			if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) )
			{
				// Select entity
				mApp->SelectEntity( e );
			}

			if ( input->IsKeyDown( KeyCode::LeftMouseButton ) )
			{
				if ( !held )
				{
					mGrabbedEntity = e;
					held = true;
				}
			}

			if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) )
			{
				Entity* c = mGrabbedEntity.Get( );
				if ( c && e != c && c->GetParent().Get() != e )
				{
					AttemptToAddChild( e, c );
				}

				held = false;
				mGrabbedEntity = EntityHandle::Invalid( );
			} 
		}

		return anyItemHovered;
	}
	*/

	bool EditorWorldOutlinerView::DisplayEntityRecursively( const EntityHandle& handle, const u32& indentionLevel )
	{
		Entity* entity = handle.Get( );
		if ( entity == nullptr )
		{
			return false;
		} 

		Input* input = EngineSubsystem( Input );

		static HashMap< u32, bool > mIsTreeOpen;

		const float indentionLevelOffset = 20.0f;
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

		// Draw triangle
		if ( entity->HasChildren( ) )
		{
			auto query = mIsTreeOpen.find( entity->GetID( ) );
			if ( query == mIsTreeOpen.end( ) )
			{
				mIsTreeOpen[ entity->GetID( ) ] = false;
			} 
		}

		// Bounding rect box hovering
		if ( ImGui::IsMouseHoveringRect( a, b ) )
		{
			dl->AddRectFilled( a, b, ImGui::GetColorU32( ImGuiCol_HeaderHovered ) );

			if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) )
			{
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

		ImGui::Text( fmt::format( "{}##{}", entity->GetName( ), entity->GetID( ) ).c_str( ) ); 

		// Display all entity children
		if ( entity->HasChildren( ) )
		{
			if ( mIsTreeOpen[ entity->GetID( ) ] )
			{
				for ( auto& c : entity->GetChildren( ) )
				{
					DisplayEntityRecursively( c, indentionLevel + 1 );
				} 
			}
		} 

		return false; 
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
			if ( !anyItemHovered )
			{
				if ( mGrabbedEntity )
				{
					if ( ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImVec2( ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y ) ) )
					{
						mGrabbedEntity.Get( )->RemoveParent( );
						mGrabbedEntity = EntityHandle::Invalid( ); 
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
