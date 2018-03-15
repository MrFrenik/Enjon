// File: EditorInspectorView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorInspectorView.h"
#include "EditorApp.h"
#include "Project.h"

#include <Engine.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>
#include <Utils/FileUtils.h>
#include <Entity/Components/RigidBodyComponent.h>

#include <filesystem>
#include <fmt/format.h>

namespace Enjon
{
	//==========================================================================

	EditorInspectorView::EditorInspectorView( EditorApp* app ) 
		: EditorView( app, "Inspector", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	}

	//==========================================================================

	void EditorInspectorView::UpdateView( )
	{ 
		if ( mApp->GetSelectedEntity( ) )
		{
			// Debug dump the entity ( Probably shouldn't do this and should tailor it more... )
			Entity* ent = mApp->GetSelectedEntity( ).Get( );

			// New component dialogue
			ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_16" ) );
			if ( ImGui::BeginCombo( "##ADDCOMPONENT", "+ Add Component..." ) )
			{
				// Label for scripting type of component class
				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_10" ) );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
				ImGui::Text( "Scripting" );
				ImGui::PopStyleColor( );
				ImGui::PopFont( );

				// Add new component pop up
				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_14" ) );
				if ( ImGui::Selectable( "\tCreate New Component..." ) )
				{
					mApp->OpenNewComponentDialogue( );
				}
				ImGui::PopFont( );

				// Separator line
				ImGui::Separator( );

				// Get component list
				EntityManager* entities = EngineSubsystem( EntityManager );
				auto compMetaClsList = entities->GetComponentMetaClassList( );

				// Label for scripting type of component class
				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_10" ) );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
				ImGui::Text( "Custom" );
				ImGui::PopStyleColor( );
				ImGui::PopFont( );

				ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_14" ) );
				for ( auto& cls : compMetaClsList )
				{
					if ( !ent->HasComponent( cls ) )
					{
						// Add component to mEntity
						if ( ImGui::Selectable( fmt::format( "\t{}", cls->GetName( ) ).c_str() ) )
						{
							ent->AddComponent( cls );
						} 
					}
				} 
				ImGui::PopFont( );

				ImGui::EndCombo( );
			}
			ImGui::PopFont( ); 

			ImGui::ListBoxHeader( "##CompLists", ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f ) );
			{
				// Transform information
				if ( ImGui::CollapsingHeader( "Transform" ) )
				{
					ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_14" ) );
					ImGuiManager::DebugDumpProperty( ent, ent->Class( )->GetPropertyByName( "mLocalTransform" ) ); 
					ImGui::PopFont( );
				} 

				for ( auto& c : ent->GetComponents( ) )
				{
					if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
					{
						MetaClass* cls = const_cast< MetaClass * > ( c->Class( ) );
						s32 shapeType = -1;
						if ( cls->InstanceOf<RigidBodyComponent>( ) )
						{
							shapeType = (s32)c->Cast<RigidBodyComponent>( )->GetShapeType( );
						}

						ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_14" ) );
						ImGuiManager::DebugDumpObject( c ); 
						ImGui::PopFont( );

						// Shape type changed
						// TODO(): Be able to hook in specific delegates for property changes through reflection system
						if ( shapeType != -1 && shapeType != (s32)c->Cast< RigidBodyComponent >()->GetShapeType() )
						{
							c->ConstCast< RigidBodyComponent >( )->SetShape( c->Cast< RigidBodyComponent >( )->GetShapeType( ) );
						} 

						if ( ImGui::Button( fmt::format( "Remove##{}", (u32)c ).c_str() ) )
						{
							ent->RemoveComponent( c->Class( ) );
						}
					}
				} 
			}
			ImGui::ListBoxFooter( );

			// Add component view popup ( if opened )
			mApp->AddComponentPopupView( );
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
}
