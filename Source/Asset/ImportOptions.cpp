// @file ImportOptions.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.  

#include "Asset/ImportOptions.h"
#include "ImGui/ImGuiManager.h"
#include "Asset/AssetLoader.h"

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

				if ( ImGui::Button( "Cancel" ) || res != Result::PROCESS_RUNNING )
				{
					mIsImporting = false;
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

