// @file ImportOptions.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.  

#include "Asset/ImportOptions.h"
#include "ImGui/ImGuiManager.h"
#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "ImGui/ImGuiManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon 
{ 
	//============================================================================

	String ImportOptions::GetResourceFilePath( ) const
	{
		return mResourceFilePath;
	}

	//============================================================================

	String ImportOptions::GetDestinationAssetDirectory( ) const
	{
		return mDestinationAssetDirectory;
	}

	//============================================================================

	bool ImportOptions::IsImporting( ) const
	{
		return mIsImporting;
	} 

	//============================================================================

	Result ImportOptions::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Just create an empty popup window for now
		if ( mIsImporting )
		{
			const char* popupName = "##ImportOptions";
			if ( !ImGui::IsPopupOpen( popupName ) )
			{
				ImGui::OpenPopup( popupName ); 
			}
			ImGui::SetNextWindowSize( ImVec2( 600.0f, 150.0f ) );
			if ( ImGui::BeginPopupModal( popupName, NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize ) )
			{
				// Display whatever in here for any derived class?
				Result res = OnEditorUIInternal( );

				// Make sure can import at this point
				if ( igm->Button( "Import" ) )
				{
					// Load asset into database
					EngineSubsystem( AssetManager )->AddToDatabase( this ); 

					// Stop importing
					mIsImporting = false;
				} 

				// Same line formatting
				igm->SameLine( );

				if ( ImGui::Button( "Cancel" ) )
				{
					// Stop importing
					mIsImporting = false;
				} 

				// Done
				if ( !mIsImporting || res != Result::PROCESS_RUNNING )
				{
					ImGui::CloseCurrentPopup( );
				}

				ImGui::EndPopup( );
			}
		}

		return Result::SUCCESS;
	} 

	//============================================================================

	const AssetLoader* ImportOptions::GetLoader( ) const
	{
		return mLoader;
	} 

	//============================================================================
}

