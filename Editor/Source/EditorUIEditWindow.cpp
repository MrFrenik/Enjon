#include "EditorUIEditWindow.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Utils/FileUtils.h>

namespace Enjon
{
	//=================================================================================

	void EditorUIEditWindow::Init( const WindowParams& params )
	{ 
		// Construct scene in world
		if ( !mUI )
		{
			// Initialize new world 
			mWorld = new World( );
			// Register contexts with world
			mWorld->RegisterContext< GraphicsSubsystemContext >( );
			// Set asset from data 
			mUI = ( UI* )( params.mData );
			// Construct Scene 
			ConstructScene( );
		} 
	}

	//=================================================================================

	void EditorUIEditWindow::ConstructScene()
	{ 
		GUIContext* guiContext = GetGUIContext( );

		// Add main menu options
		guiContext->RegisterMainMenu( "File" ); 
		guiContext->RegisterMainMenu( "Random" ); 

		World* world = GetWorld( ); 

		guiContext->RegisterWindow( "Properties", [ & ]
		{
			if ( ImGui::BeginDock( "Properties" ) )
			{
				ImGui::ListBoxHeader( "##asset_props", ImVec2( ImGui::GetWindowWidth() - 20.f, ImGui::GetWindowHeight() - 15.f ) );
				{
					if ( mUI )
					{ 
						World* world = GetWorld( );
						ImGui::Text( "     UI:" ); ImGui::SameLine(); ImGui::SetCursorPosX( ImGui::GetWindowWidth() * 0.4f );
						ImGui::Text( "%s", Utils::format( "%s", mUI.Get( )->GetName().c_str() ).c_str( ) );
						ImGuiManager* igm = EngineSubsystem( ImGuiManager );
						igm->InspectObject( mUI.Get() ); 
					} 
				}
				ImGui::ListBoxFooter();
				ImGui::EndDock( );
			}
		} );

		auto saveAssetOption = [ & ] ( )
		{
			if ( ImGui::MenuItem( "Save##save_asset_option", NULL ) )
			{
				if ( mUI )
				{
					mUI->Save();
				}
			}
		};

		// Register menu options
		guiContext->RegisterMenuOption( "File", "Save##save_asset_option", saveAssetOption ); 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Properties", nullptr, GUIDockSlotType::Slot_Tab, 0.45f ) );
		guiContext->SetActiveDock( "Properties" );
		guiContext->Finalize( ); 
	}

	//=================================================================================

}
