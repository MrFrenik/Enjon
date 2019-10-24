// File: EditorDocumentationView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorDocumentationView.h"
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

	EditorDocumentationView::EditorDocumentationView( EditorApp* app, Window* window ) 
		: EditorView( app, window, "Documentation", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	} 

	//==========================================================================

	void EditorDocumentationView::ViewClass( const MetaClass* cls )
	{
		// How to view? Let's just show the class name for now
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		if ( cls )
		{ 
			ImGui::ListBoxHeader( "##cls", ImVec2( ImGui::GetWindowWidth() * 0.975f, ImGui::GetWindowHeight() * 0.965f ) );
			{
				if ( ImGui::Button( "Back" ) )
				{
					DeselectInspectedCls( );
					//return;
				}

				igm->PushFont( "WeblySleek_24" );
				{
					ImGui::Text( "%s", cls->GetName().c_str() );
				}

				igm->PopFont( );

				// Formatting
				ImGui::NewLine( ); 
				ImGui::SetCursorPosX( 40.f );
				ImGui::Text( "Description: " );

				// Formatting
				ImGui::NewLine( ); 

				ImGui::SetCursorPosX( 40.f );
				ImGui::Text( "Properties: " );
				ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 10.f );
				for ( auto& prop : cls->GetProperties( ) )
				{
					ImGui::SetCursorPosX( 80.f );
					ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.8f, 0.1f, 0.2f, 1.f ) );
					ImGui::Text( "%s", prop->GetTypeStr() );
					ImGui::SameLine( );
					ImGui::PopStyleColor( );
					ImGui::SetCursorPosX( 180.f );
					ImGui::Text( "%s", prop->GetName().c_str() );
				}

				// Formatting
				ImGui::NewLine( ); 
				ImGui::SetCursorPosX( 40.f );
				ImGui::Text( "Functions: " );
				for ( auto& f : cls->GetFunctions( ) )
				{
					ImGui::SetCursorPosX( 80.f );
					ImGui::Text( "%s", f.second->GetName( ).c_str() );
				} 
			}
			ImGui::ListBoxFooter();
		}
	}

	//==========================================================================

	void EditorDocumentationView::UpdateView( )
	{
		switch ( mInspectedClsID )
		{
			// Main view
			case -1: 
			{ 
				// Show main view for documentation by default
				const HashMap< u32, MetaClass* >& registry = Engine::GetInstance( )->GetMetaClassRegistry( )->GetRegistry(); 

				ImGui::ListBoxHeader( "##registry", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 20.f ) );
				{
					for ( auto& c : registry )
					{
						const MetaClass* cls = c.second;
						if ( ImGui::Selectable( cls->GetName( ).c_str( ) ) )
						{ 
							mInspectedClsID = cls->GetTypeId( );
						}
					} 
				}
				ImGui::ListBoxFooter( );
			} break;

			// Detailed view for class
			default: 
			{ 
				const MetaClass* cls = Object::GetClass( mInspectedClsID );
				if ( cls ) {
					ViewClass( Object::GetClass( mInspectedClsID ) ); 
				}
				else {
					DeselectInspectedCls( );
				}
			} break;
		}
	}

	//==========================================================================

	void EditorDocumentationView::ProcessViewInput( )
	{ 
	}

	//==========================================================================

	void EditorDocumentationView::Initialize( )
	{ 
	} 

	//=========================================================================

	void EditorDocumentationView::SetInspetedClassID( const s32& clsID )
	{
		mInspectedClsID = clsID;
	}

	//=========================================================================

	void EditorDocumentationView::DeselectInspectedCls( )
	{
		mInspectedClsID = -1;
	}

	//=========================================================================
}
