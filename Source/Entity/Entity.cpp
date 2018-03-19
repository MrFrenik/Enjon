// File: Entity.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/EntityManager.h"
#include "ImGui/ImGuiManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

#include <fmt/format.h>

namespace Enjon 
{
	//=====================================================================

	Result Entity::OnEditorUI( )
	{
		// Grab imgui manager
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Display all components and transform information
		ImGui::ListBoxHeader( "##CompLists", ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f ) );
		{
			// Transform information
			if ( ImGui::CollapsingHeader( "Transform" ) )
			{
				ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
				igm->DebugDumpProperty( this, Class( )->GetPropertyByName( "mLocalTransform" ) ); 
				ImGui::PopFont( );
			} 

			for ( auto& c : GetComponents( ) )
			{
				if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
				{
					ImGui::PushFont( igm->GetFont( "WeblySleek_14" ) );
					igm->InspectObject( c ); 
					ImGui::PopFont( );

					if ( ImGui::Button( fmt::format( "Remove##{}", (u32)c ).c_str() ) )
					{
						RemoveComponent( c->Class( ) );
					}
				}
			} 
		}
		ImGui::ListBoxFooter( );

		return Result::SUCCESS; 
	} 

	//=====================================================================
}