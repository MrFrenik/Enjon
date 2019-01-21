// File: EditorWorldOutlinerView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorWorldOutlinerView.h"
#include "EditorMaterialEditWindow.h"
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

	EditorWorldOutlinerView::EditorWorldOutlinerView( EditorApp* app, Window* window )
		: EditorView( app, window, "World Outliner", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	} 

	//=================================================================

	void EditorWorldOutlinerView::Initialize( )
	{
	}

	//=================================================================

	void EditorWorldOutlinerView::RegisterEntitySelectionCallback( const EntitySelectionCallback& callback )
	{
		mEntitySelectionCallbacks.push_back( callback );
	} 

	//=================================================================

	void EditorWorldOutlinerView::RegisterEntityDeselectionCallback( const EntityDeselectionCallback& callback )
	{
		mEntityDeselectionCallbacks.push_back( callback );
	}

	//=================================================================

	EntityHandle EditorWorldOutlinerView::GetSelectedEntity( )
	{
		return mSelectedEntity;
	}

	//================================================================= 

	void EditorWorldOutlinerView::DeselectEntity( )
	{ 
		for ( auto& f : mEntityDeselectionCallbacks )
		{
			f( );
		}

		mSelectedEntity = EntityHandle::Invalid( );
	} 

	//================================================================= 

	void EditorWorldOutlinerView::SelectEntity( const EntityHandle& entity )
	{
		mSelectedEntity = entity;

		// Callbacks for selected entity
		for ( auto& f : mEntitySelectionCallbacks )
		{
			f( mSelectedEntity );
		}
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
		if ( child->HasChild( parent ) )
		{
			return false;
		}

		// If child has parent, then un-parent it
		if ( child->HasParent( ) && child->GetParent( ) == parent )
		{
			child->RemoveParent( );
		}
		else
		{ 
			// Recursively determine if parent is in child's hiearchy already
			bool isInHierarchy = IsParentInChildHierarchy( child, parent );

			if ( !isInHierarchy )
			{
				if ( child->HasParent( ) )
				{
					child->RemoveParent( );
				}

				parent->AddChild( child );
			} 
		}

		return true;
	}

	bool CanParentTo( const EntityHandle& parent, const EntityHandle& child )
	{
		return ( ( parent.Get( ) != child.Get( ) ) && !( IsParentInChildHierarchy( child.Get( ), parent.Get( ) ) ) );
	}

	String GetAttachmentLabel( const EntityHandle& parent, const EntityHandle& child )
	{
		String label = "";

		Entity* pEnt = parent.Get( );
		Entity* cEnt = child.Get( );

		// Cannot be null
		if ( pEnt == nullptr || cEnt == nullptr )
		{
			return "";
		}

		// If they're the same
		if ( pEnt == cEnt )
		{
			label = fmt::format( "Cannot attach {} to self.", cEnt->GetName() );
		}

		// Is parent
		else if ( cEnt->GetParent( ) == pEnt ) 
		{
			label = fmt::format( "Detach {} from {}.", cEnt->GetName( ), pEnt->GetName( ) );
		}

		else if ( CanParentTo( parent, child ) )
		{
			label = fmt::format( "Attach {} to {}.", cEnt->GetName(), pEnt->GetName() ); 
		}
		else
		{
			label = fmt::format( "Cannot attach {} to {}.", cEnt->GetName(), pEnt->GetName() ); 
		} 

		return label; 
	}

	bool EditorWorldOutlinerView::DisplayEntityRecursively( const EntityHandle& handle, u32* entityNumber, const u32& indentionLevel )
	{ 
		Entity* entity = handle.Get( );
		if ( entity == nullptr )
		{
			return false;
		} 

		EntityHandle selectedHandle = mSelectedEntity;
		Entity* selectedEnt = selectedHandle.Get( );
		Input* input = EngineSubsystem( Input );

		// Whether or not the entity is selected
		bool selected = entity == selectedEnt;
		// Whether or not the entity text is hovered
		bool hovered = false;

		// boolean to return
		bool anyItemHovered = false; 

		const float indentionLevelOffset = 10.0f;
		const float boxIndentionLevelOffset = 20.0f;
		auto dl = ImGui::GetWindowDrawList( );
		const float rXO = 5.0f;
		bool validParentOption = true;

		ImVec2 a, b; 

		// Capture a
		a = ImVec2( ImGui::GetWindowPos().x + rXO, ImGui::GetCursorScreenPos( ).y );

		// Entity label text for name
		String entityLabelText = entity->GetName( );

		// Calculate label text size for bounding box
		ImVec2 textSize = ImGui::CalcTextSize( entityLabelText.c_str( ) ); 

		// Capture b
		ImVec2 selectionB = ImVec2( a.x + ImGui::GetWindowSize().x * 0.7f - rXO - 5.0f, a.y + textSize.y );
		b = ImVec2( a.x + ImGui::GetWindowSize().x - rXO - 5.0f, a.y + textSize.y );

		// Display entity name at indention level
		ImGui::SetCursorPosX( ImGui::GetCursorPosX() + indentionLevel * indentionLevelOffset ); 

		// Set hovered
		hovered = ImGui::IsMouseHoveringRect( a, selectionB );

		// Get UUID string
		String entityUUIDStr = entity->GetUUID( ).ToString( );

		// Add background if hovered or selected
		if ( hovered || selected )
		{
			ImColor hoveredColor = selected ? ImGui::GetColorU32( ImGuiCol_ListSelectionActive ) :  ImGui::GetColorU32( ImGuiCol_ListSelectionHovered );
			if ( !selected )
			{
				hoveredColor.Value.w = 0.4f; 
			}

			// If name change entity
			if ( mEntityNameChangeID == entity->GetID( ) )
			{
				hoveredColor = ImGui::GetColorU32( ImGuiCol_ListSelectionRenamed );
				b.y += 5.0f;
			}

			dl->AddRectFilled( a, b, hoveredColor ); 
		}

		// Debug draw rect
		dl->AddRectFilled( a, b, *entityNumber % 2 == 0 ? ImColor( 0.5f, 0.5f, 0.5f, 0.05f ) : ImColor( 1.0f, 1.0f, 1.0f, 0.0f ) );
		*entityNumber = *entityNumber + 1; 

		// Draw triangle
		bool boxSelected = false;
		if ( entity->HasChildren( ) )
		{ 
			auto query = mIsTreeOpen.find( entityUUIDStr );
			if ( query == mIsTreeOpen.end( ) )
			{
				mIsTreeOpen[ entityUUIDStr ] = false;
			} 

			bool isOpen = mIsTreeOpen[ entityUUIDStr ]; 

			// Draw box for tree 
			float boxSize = 8.0f;
			ImVec2 ba = ImVec2( ImGui::GetCursorScreenPos( ).x + boxSize / 2.0f, ImGui::GetCursorScreenPos( ).y + textSize.y / 2.0f - boxSize / 2.0f );
			ImVec2 bb = ImVec2( ba.x + boxSize, ba.y + boxSize );

			bool hoveringCollapseTriangle = ImGui::IsMouseHoveringRect( ba, bb );

			// Draw open triangle
			if ( isOpen )
			{
				if ( hoveringCollapseTriangle )
				{
					ImColor triangleColor = ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
					if ( input->IsKeyDown( KeyCode::LeftMouseButton ) )
					{ 
						float diff = 2.0f;
						dl->AddTriangleFilled( ImVec2( bb.x, bb.y + diff ), ImVec2( ba.x, bb.y + diff ), ImVec2( bb.x, ba.y + diff ), triangleColor ); 
					}
					else
					{
						dl->AddTriangleFilled( bb, ImVec2( ba.x, bb.y ), ImVec2( bb.x, ba.y ), triangleColor ); 
					}
				}
				else
				{
					dl->AddTriangle( bb, ImVec2( ba.x, bb.y ), ImVec2( bb.x, ba.y ), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) ); 
				}
			}
			// Draw collapsed triangle
			else
			{
				if ( hoveringCollapseTriangle )
				{
					if ( input->IsKeyDown( KeyCode::LeftMouseButton ) )
					{
						float diff = 2.0f;
						dl->AddTriangleFilled( ImVec2( ba.x, ba.y + diff ), ImVec2( ba.x, bb.y + diff ), ImVec2( bb.x, ( ba.y + bb.y ) / 2.0f + diff ), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) ); 
					}
					else
					{
						dl->AddTriangleFilled( ba, ImVec2( ba.x, bb.y ), ImVec2( bb.x, ( ba.y + bb.y ) / 2.0f ), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) ); 
					}
				}
				else
				{
					dl->AddTriangle( ba, ImVec2( ba.x, bb.y ), ImVec2( bb.x, ( ba.y + bb.y ) / 2.0f ), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) ); 
				}
			}

			if ( hoveringCollapseTriangle )
			{
				if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) ) 
				{
					boxSelected = true;
					auto query = mIsTreeOpen.find( entityUUIDStr );
					if ( query == mIsTreeOpen.end( ) )
					{
						mIsTreeOpen[ entityUUIDStr ] = true;
					}
					else
					{
						mIsTreeOpen[ entityUUIDStr ] = !mIsTreeOpen[ entityUUIDStr ]; 
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
			// Select entity
			if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) && !boxSelected )
			{
				//mApp->SelectEntity( entity ); 

				SelectEntity( entity );

				// Stop name changing 
				if ( mEntityNameChangeID != entity->GetID( ) )
				{
					mEntityNameChangeID = EntityHandle::Invalid( ).GetID( );
				}
			} 

			if ( ImGui::IsMouseDown( 0 ) )
			{ 
				if ( !mMouseHeld )
				{
					mHeldMousePosition = input->GetMouseCoords( );
					//mGrabbedEntity = entity;
					mMouseHeld = true;
				}
				else if ( mMouseHeld && !mGrabbedEntity )
				{
					if ( mHeldMousePosition != input->GetMouseCoords() && mHeldMousePosition.Distance(input->GetMouseCoords()) >= 2.0f )
					{ 
						mGrabbedEntity = entity;
					} 
				}
			}

			// Label for attachment
			if ( ImGui::IsMouseDown( 0 ) && mGrabbedEntity )
			{
				String label = GetAttachmentLabel( entity, mGrabbedEntity );
				ImVec2 txtSize = ImGui::CalcTextSize( label.c_str( ) );
				ImGui::SetNextWindowPos( ImVec2( ImGui::GetMousePos( ).x + 15.0f, ImGui::GetMousePos().y + 5.0f ) );
				ImGui::SetNextWindowSize( ImVec2( txtSize.x + 20.0f, txtSize.y ) );
				ImGui::Begin( "##window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
				{
					ImGui::Text( label.c_str( ) );
				}
				ImGui::End( );
			}

			if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) )
			{
				Entity* c = mGrabbedEntity.Get( );
				if ( c && entity != c )
				{
					bool added = AttemptToAddChild( entity, c ); 
					if ( added )
					{
						
						auto query = mIsTreeOpen.find( entityUUIDStr );
						if ( query == mIsTreeOpen.end( ) )
						{
							mIsTreeOpen[ entityUUIDStr ] = true;
						}
					}
				}

				mMouseHeld = false;
				mGrabbedEntity = EntityHandle::Invalid( );
				mHeldMousePosition = Vec2( -1, -1 );
			} 
		}

		auto colors = ImGui::GetStyle( ).Colors;
		ImColor textColor = mGrabbedEntity ? CanParentTo( entity, mGrabbedEntity ) ? colors[ ImGuiCol_Text ] : colors[ ImGuiCol_TextDisabled ] : entity->HasPrototypeEntity() ? colors[ImGuiCol_SeparatorHovered] : colors[ ImGuiCol_Text ]; 

		// Display label text
		if ( entity->GetID( ) == mEntityNameChangeID )
		{ 
			ImGui::SetKeyboardFocusHere( -1 );

			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			char buffer[ 256 ];
			String name = entity->GetName( );
			std::strncpy( buffer, name.c_str(), 256 );
			if ( ImGui::InputText( "##", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll ) )
			{
				entity->SetName( buffer );
				mEntityNameChangeID = EntityHandle::Invalid( ).GetID( );
			}
			ImGui::PopStyleColor( 2 ); 
		}
		else
		{
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( textColor ) );
			ImGui::Text( entityLabelText.c_str() ); 
			ImGui::PopStyleColor( ); 
		}
 
		// If is an archetype, display options for that
		if ( entity->GetArchetype( ) )
		{
			ImGui::SameLine( );

			String label = "View Archetype";
			f32 framePadding = 20.0f;
			ImVec2 ts = ImGui::CalcTextSize( label.c_str( ) );
			ImGui::SetCursorPosX( ImGui::GetWindowWidth( ) - ts.x - framePadding );
			ImVec2 a = ImVec2( ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y );
			ImVec2 b = ImVec2( a.x + ts.x, a.y + ts.y );
			ImGui::PushClipRect( a, b, true );
			ImGui::TextColored( ImGui::IsMouseHoveringRect( a, b ) ? ImColor( ImGui::GetColorU32( ImGuiCol_SeparatorActive ) ) : ImColor( ImGui::GetColorU32( ImGuiCol_TextDisabled ) ), label.c_str( ) );
			ImGui::PopClipRect( );
			if ( ImGui::IsMouseClicked( 0 ) && ImGui::IsMouseHoveringRect( a, b ) )
			{
				// Open archetype window
				const Asset* archType = entity->GetArchetype( ).Get( );

				// Open new edit window for this archetype
				WindowParams params;
				params.mWindow = new EditorArchetypeEditWindow( archType );
				params.mName = archType->GetName( );
				params.mWidth = 1200;
				params.mHeight = 800;
				params.mFlags = WindowFlagsMask( ( u32 )WindowFlags::RESIZABLE );
				Window::AddNewWindow( params ); 
			}
		}

		// Display all entity children
		if ( entity->HasChildren( ) )
		{
			if ( mIsTreeOpen[ entityUUIDStr ] )
			{
				for ( auto& c : entity->GetChildren( ) )
				{ 
					anyItemHovered |= DisplayEntityRecursively( c, entityNumber, indentionLevel + 1 );
				} 
			}
		} 

		return ( anyItemHovered | hovered ); 
	}

	void EditorWorldOutlinerView::UpdateView( )
	{
		ImDrawList* dl = ImGui::GetWindowDrawList( ); 

		EntityManager* entities = EngineSubsystem( EntityManager );

		Input* input = EngineSubsystem( Input ); 

		// List out active entities
		Vec2 padding( 20.0f, 40.0f );
		f32 height = ImGui::GetWindowSize( ).y - ImGui::GetCursorPosY( ) - padding.y;
		bool anyItemHovered = false;
		u32 entityNumber = 0;
		ImGui::ListBoxHeader( "##EntitiesListWorldOutliner", ImVec2( ImGui::GetWindowSize( ).x - padding.x, height ) );
		{
			//EntityHandle selectedEntityHandle = mApp->GetSelectedEntity( );
			EntityHandle selectedEntityHandle = GetSelectedEntity( );
			Entity* selectedEntity = selectedEntityHandle.Get( );
			for ( auto& e : entities->GetEntitiesByWorld( mWindow->GetWorld( ) ) )
			{ 
				// SKip over non-root entities
				if ( e->HasParent( ) )
				{
					continue;
				} 

				// Display entity
				anyItemHovered |= DisplayEntityRecursively( e, &entityNumber ); 
			} 
		}
		ImGui::ListBoxFooter( ); 

		ImVec2 selectionRectA = ImVec2( ImGui::GetWindowPos( ) );
		ImVec2 selectionRectB = ImVec2( selectionRectA.x + ImGui::GetWindowSize( ).x * 0.7f, selectionRectA.y + ImGui::GetWindowSize( ).y );
		if ( ImGui::IsMouseHoveringRect( ImGui::GetWindowPos( ), ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetWindowSize( ).x, ImGui::GetWindowPos( ).y + ImGui::GetWindowSize( ).y ) ) )
		{
			if ( !anyItemHovered )
			{
				// Attachment label
				if ( input->IsKeyDown( KeyCode::LeftMouseButton ) )
				{
					if ( mGrabbedEntity )
					{
						String label = fmt::format( "Detach {}.", mGrabbedEntity.Get( )->GetName( ) );
						ImVec2 txtSize = ImGui::CalcTextSize( label.c_str( ) );
						ImGui::SetNextWindowPos( ImVec2( ImGui::GetMousePos( ).x + 15.0f, ImGui::GetMousePos().y + 5.0f ) );
						ImGui::SetNextWindowSize( ImVec2( txtSize.x + 20.0f, txtSize.y ) );
						ImGui::Begin( "##window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
						{
							ImGui::Text( label.c_str( ) );
						}
						ImGui::End( ); 
					}
				}
				else if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) || ImGui::IsMouseReleased( 0 ) )
				{
					if ( mGrabbedEntity )
					{
						mGrabbedEntity.Get( )->RemoveParent( );
						mGrabbedEntity = EntityHandle::Invalid( ); 
						mHeldMousePosition = Vec2( -1, -1 );
					}
					else
					{
						DeselectEntity( );
						//mApp->DeselectEntity( );
						mEntityNameChangeID = EntityHandle::Invalid( ).GetID( );
					}
				}
			}
		} 

		if ( input->IsKeyReleased( KeyCode::LeftMouseButton ) )
		{
			if ( mGrabbedEntity )
			{ 
				mGrabbedEntity = EntityHandle::Invalid( ); 
				mHeldMousePosition = Vec2( -1, -1 );
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
		ImGui::Text( fmt::format( "{} Entities", entities->GetEntitiesByWorld( mWindow->GetWorld( ) ).size() ).c_str( ) );
		ImGui::PopFont( );
		ImGui::PopStyleColor( );

		if ( input->IsKeyPressed( KeyCode::F2 ) && mSelectedEntity )
		{
			mEntityNameChangeID = mSelectedEntity.Get( )->GetID( );
		} 

		ImGui::SameLine( );
		ImGui::Text( "Frame: %.3f", 1000.0f / ( f32 )ImGui::GetIO( ).Framerate );
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

		// Capture hovered state
		mIsHovered = ImGui::IsWindowHovered( ); 

		// Capture focused state
		mIsFocused = ( mIsHovered && ( input->IsKeyDown( KeyCode::LeftMouseButton ) || input->IsKeyDown( KeyCode::RightMouseButton ) ) );
	} 

	//==================================================================
}
