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

	//void EditorInspectorView::UpdateView( )
	//{ 
	//	if ( mApp->GetSelectedEntity( ) )
	//	{
	//		// Debug dump the entity ( Probably shouldn't do this and should tailor it more... )
	//		Entity* ent = mApp->GetSelectedEntity( ).Get( );

	//		// Get ImGuiManager subsystem
	//		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

	//		// New component dialogue
	//		ImGui::PushFont( igm->GetFont( "WeblySleek_16" ) );
	//		if ( ImGui::BeginCombo( "##ADDCOMPONENT", "+ Add Component..." ) )
	//		{
	//			// Label for scripting type of component class
	//			ImGui::PushFont( igm->GetFont( "WeblySleek_10" ) );
	//			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
	//			ImGui::Text( "Scripting" );
	//			ImGui::PopStyleColor( );
	//			ImGui::PopFont( );

	//			// Add new component pop up
	//			ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
	//			if ( ImGui::Selectable( "\tCreate New Component..." ) )
	//			{
	//				mApp->OpenNewComponentDialogue( );
	//			}
	//			ImGui::PopFont( );

	//			// Separator line
	//			ImGui::Separator( );

	//			// Get component list
	//			EntityManager* entities = EngineSubsystem( EntityManager );
	//			auto compMetaClsList = entities->GetComponentMetaClassList( );

	//			// Label for scripting type of component class
	//			ImGui::PushFont( igm->GetFont( "WeblySleek_10" ) );
	//			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
	//			ImGui::Text( "Custom" );
	//			ImGui::PopStyleColor( );
	//			ImGui::PopFont( );

	//			ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
	//			for ( auto& cls : compMetaClsList )
	//			{
	//				if ( !ent->HasComponent( cls ) )
	//				{
	//					// Add component to mEntity
	//					if ( ImGui::Selectable( fmt::format( "\t{}", cls->GetName( ) ).c_str() ) )
	//					{
	//						ent->AddComponent( cls );
	//					} 
	//				}
	//			} 
	//			ImGui::PopFont( );

	//			ImGui::EndCombo( );
	//		}
	//		ImGui::PopFont( ); 

	//		ImGui::ListBoxHeader( "##CompLists", ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f ) );
	//		{
	//			// Transform information
	//			if ( ImGui::CollapsingHeader( "Transform" ) )
	//			{
	//				ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
	//				igm->DebugDumpProperty( ent, ent->Class( )->GetPropertyByName( "mLocalTransform" ) ); 
	//				ImGui::PopFont( );
	//			} 

	//			for ( auto& c : ent->GetComponents( ) )
	//			{
	//				if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
	//				{
	//					MetaClass* cls = const_cast< MetaClass * > ( c->Class( ) );
	//					s32 shapeType = -1;
	//					if ( cls->InstanceOf<RigidBodyComponent>( ) )
	//					{
	//						shapeType = (s32)c->Cast<RigidBodyComponent>( )->GetShapeType( );
	//					}

	//					ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
	//					igm->InspectObject( c ); 
	//					ImGui::PopFont( );

	//					// Shape type changed
	//					// TODO(): Be able to hook in specific delegates for property changes through reflection system
	//					if ( shapeType != -1 && shapeType != (s32)c->Cast< RigidBodyComponent >()->GetShapeType() )
	//					{
	//						c->ConstCast< RigidBodyComponent >( )->SetShape( c->Cast< RigidBodyComponent >( )->GetShapeType( ) );
	//					} 

	//					if ( ImGui::Button( fmt::format( "Remove##{}", (u32)c ).c_str() ) )
	//					{
	//						ent->RemoveComponent( c->Class( ) );
	//					}
	//				}
	//			} 
	//		}
	//		ImGui::ListBoxFooter( );

	//		// Add component view popup ( if opened )
	//		mApp->AddComponentPopupView( );
	//	}

	//	// No selected entity
	//	else
	//	{
	//		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImColor( ImGui::GetColorU32( ImGuiCol_TextDisabled ) ) ) );
	//		ImVec2 centerPos = ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetWindowWidth() / 2.0f, ImGui::GetWindowPos( ).y + ImGui::GetWindowHeight() / 2.0f );
	//		const char* text = "Select Object to Inspect.";
	//		ImVec2 textSize = ImGui::CalcTextSize( text );
	//		centerPos.x -= textSize.x / 2.0f;
	//		centerPos.y -= textSize.y / 2.0f;
	//		ImGui::SetCursorScreenPos( centerPos );
	//		ImGui::Text( text );
	//		ImGui::PopStyleColor( );
	//	}


	//}

	//==========================================================================

	void EditorInspectorView::InspectEntityViewHeader( Entity* ent )
	{
		 //Get ImGuiManager subsystem
		ImGuiManager* igm = EngineSubsystem( ImGuiManager ); 

		// New component dialogue
		ImGui::PushFont( igm->GetFont( "WeblySleek_16" ) );
		if ( ImGui::BeginCombo( "##ADDCOMPONENT", "+ Add Component..." ) )
		{
			// Label for scripting type of component class
			ImGui::PushFont( igm->GetFont( "WeblySleek_10" ) );
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
			ImGui::PushFont( igm->GetFont( "WeblySleek_10" ) );
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

			ImGuiManager* igm = EngineSubsystem( ImGuiManager );
			igm->InspectObject( mInspectedObject ); 

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
			ImGui::Text( text );
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

	void EditorInspectorView::DeselectInspectedObject( const Object* object )
	{
		// Deselect iff the inspected object is the object being inspected
		if ( mInspectedObject == object )
		{
			mInspectedObject = nullptr;
		}
	}

	//=========================================================================
}
