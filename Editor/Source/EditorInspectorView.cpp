// File: EditorInspectorView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorInspectorView.h"
#include "EditorApp.h"
#include "Project.h"

#include <Engine.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>
#include <Serialize/EntityArchiver.h>
#include <Utils/FileUtils.h>
#include <Entity/Components/RigidBodyComponent.h>
#include <Serialize/ObjectArchiver.h>

#include <fs/filesystem.hpp>

namespace Enjon
{
	//==========================================================================

	EditorInspectorView::EditorInspectorView( EditorApp* app, Window* window ) 
		: EditorView( app, window, "Inspector", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	}

	//==========================================================================

	void EditorInspectorView::InspectEntityViewHeader( Entity* ent )
	{
		 //Get ImGuiManager subsystem
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 


		// New component dialogue
		ImColor textCol = ImGui::GetColorU32( ImGuiCol_Text );
		ImGui::PushFont( igm->GetFont( "Roboto-MediumItalic_14" ) );
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImColor( ImGui::GetColorU32( ImGuiCol_TextDisabled ) ) ) );
		if ( ImGui::BeginCombo( "##ADDCOMPONENT", "+ Add Component..." ) )
		{
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( textCol ) );
			{ 
				// Label for scripting type of component class
				ImGui::PushFont( igm->GetFont( "Roboto-MediumItalic_12" ) );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
				ImGui::Text( "Scripting" );
				ImGui::PopStyleColor( );
				ImGui::PopFont( );

				// Add new component pop up
				ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
				if ( ImGui::Selectable( "\tCreate New Component..." ) )
				{
					mApp->EnableOpenNewComponentDialogue( );
				}
				ImGui::PopFont( );

				// Separator line
				ImGui::Separator( );

				// Get component list
				EntityManager* entities = EngineSubsystem( EntityManager );
				auto compMetaClsList = entities->GetComponentMetaClassList( );

				// Label for scripting type of component class
				ImGui::PushFont( igm->GetFont( "Roboto-MediumItalic_12" ) );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
				ImGui::Text( "Custom" );
				ImGui::PopStyleColor( );
				ImGui::PopFont( );

				ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
				for ( auto& cls : compMetaClsList )
				{
					if ( !ent->HasComponent( cls ) )
					{
						// Add component to mEntity
						if ( ImGui::Selectable( Utils::format( "\t%s", cls->GetName().c_str() ).c_str() ) )
						{
							ent->AddComponent( cls );
						} 
					}
				} 
				ImGui::PopFont( );
			}
			ImGui::PopStyleColor( );

			ImGui::EndCombo( );
		}
		ImGui::PopStyleColor( );
		ImGui::PopFont( ); 

		// Attempt to open new component dialogue box
		mApp->OpenNewComponentDialogue( );
	}

	//==========================================================================

	void EditorInspectorView::UpdateView( )
	{
		// If object is available, then inspect it
		if ( mInspectedObject )
		{
			if ( const_cast< MetaClass*>( mInspectedObject->Class( ) )->InstanceOf< Entity >( ) )
			{
				InspectEntityViewHeader( (Entity*)mInspectedObject );
			}

			// Record property overrides after editing 
			Entity* ent = mInspectedObject->ConstCast< Entity >( );

			// Clear overrides if no prototype entity exists
			if ( ent && !ent->GetPrototypeEntity( ) )
			{
				// Shouldn't have any property overrides, so just clear any that might be around
				// NOTE(John): Getting pretty damn expensive at this point...
				ent->ClearAllPropertyOverrides( ); 
			} 
 
			// Don't really like this solution, but ya know...
			ByteBuffer before, after;
			EntityArchiver::Serialize( EntityHandle( ( Entity* )mInspectedObject ), &before ); 

			ImGuiManager* igm = EngineSubsystem( ImGuiManager );
			igm->InspectObject( mInspectedObject ); 

			// Serialize after
			EntityArchiver::Serialize( EntityHandle( ( Entity* )mInspectedObject ), &after );

			// Compare the contents for changes
			bool changeOccured = !ByteBuffer::ContentsEqual( before, after ); 

			// If prototype entity exists
			if ( ent && ent->GetPrototypeEntity() ) 
			{
				if ( changeOccured )
				{ 
					ObjectArchiver::RecordAllPropertyOverrides( ent->GetPrototypeEntity().Get(), ent ); 
				} 
			}
		}
		else
		{
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImColor( ImGui::GetColorU32( ImGuiCol_TextDisabled ) ) ) );
			ImVec2 centerPos = ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetWindowWidth() / 2.0f, ImGui::GetWindowPos( ).y + ImGui::GetWindowHeight() / 2.0f );
			const char* text = "Select Object to Inspect.";
			ImVec2 textSize = ImGui::CalcTextSize( text );
			centerPos.x -= textSize.x / 2.0f;
			centerPos.y -= textSize.y / 2.0f;
			ImGui::SetCursorScreenPos( centerPos );
			ImGui::Text( "%s", text );
			ImGui::PopStyleColor( ); 
		}
	}

	//==========================================================================

	void EditorInspectorView::ProcessViewInput( )
	{ 
	}

	//==========================================================================

	void EditorInspectorView::Initialize( )
	{ 
	} 

	//=========================================================================

	void EditorInspectorView::SetInspetedObject( const Object* object )
	{
		mInspectedObject = object;
	}

	//=========================================================================

	void EditorInspectorView::DeselectInspectedObject( )
	{
		// Deselect iff the inspected object is the object being inspected
		mInspectedObject = nullptr;
	}

	//=========================================================================
}
