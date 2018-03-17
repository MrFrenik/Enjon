// File: EditorAssetBrowserView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorAssetBrowserView.h"
#include "EditorApp.h"
#include "Project.h"

#include <Engine.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>
#include <Asset/AssetManager.h>
#include <Utils/FileUtils.h>

#include <filesystem>
#include <fmt/format.h>
 
namespace FS = std::experimental::filesystem; 

namespace Enjon
{
	//=========================================================================

	EditorAssetBrowserView::EditorAssetBrowserView( EditorApp* app )
		: EditorView( app, "Asset Browser", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
	{ 
	}

	//=========================================================================

	void EditorAssetBrowserView::InitializeCurrentDirectory( Project* project )
	{ 
		// If not empty, then return
		if ( mCurrentDirectory.compare( "" ) != 0 )
		{
			return;
		}

		// If application is valid, set project path
		Application* app = project->GetApplication( );
		if ( app )
		{
			// Set current directory to project's asset directory
			mCurrentDirectory = FS::path(project->GetProjectPath( ) + "Assets").string();
			// Set root directory to be the root
			mRootDirectory = mCurrentDirectory;
		} 
	}

	//=========================================================================

	void EditorAssetBrowserView::AttemptDirectoryBackTraversal( )
	{
		// Can go backwards iff not the root directory
		FS::path current = FS::path( mCurrentDirectory );
		FS::path root = FS::path( mRootDirectory );
		if ( current == root )
		{
			return;
		}

		// Set to parent path of directory
		mCurrentDirectory = FS::path( mCurrentDirectory ).parent_path().string();
	}

	//=========================================================================

	void EditorAssetBrowserView::SetSelectedPath( const String& path )
	{
		mSelectedPath = path;
	}

	//=========================================================================

	void EditorAssetBrowserView::UpdateView( )
	{ 
		// Get editor widget manager
		EditorWidgetManager* wm = mApp->GetEditorWidgetManager( ); 

		// Get input system
		Input* input = EngineSubsystem( Input );

		// Iterate through directory structure of application 
		Project* project = mApp->GetProject( ); 

		if ( project && project->GetApplication() )
		{
			// Attempt to initialize current directory if not already ( NOTE(): hate this )
			InitializeCurrentDirectory( project );

			Application* app = project->GetApplication( );

			// Go back a directory...So fancy
			if ( ImGui::Button( "<-" ) )
			{
				AttemptDirectoryBackTraversal( );
			}

			// Display the current directory path
			ImGui::Text( fmt::format( "Current Dir: {}", mCurrentDirectory ).c_str( ) );

			// Separator for formatting
			ImGui::Separator( );

			// Iterate the current directory
			ImVec2 listBoxMin = ImGui::GetCursorScreenPos( );
			ImVec2 listBoxSize = ImVec2( ImGui::GetWindowWidth( ) - 20.0f, ImGui::GetWindowHeight( ) - 80.0f );
			bool hoveringItem = false;
			ImGui::ListBoxHeader( "##AssetDirectory", listBoxSize );
			{
				for ( auto& p : FS::directory_iterator( mCurrentDirectory ) )
				{
					// Select directory
					if ( ImGui::Selectable( p.path( ).string( ).c_str( ), p == FS::path( mSelectedPath ) ) )
					{
						SetSelectedPath( p.path( ).string( ) );
					}

					if ( ImGui::IsItemHovered( ) )
					{
						hoveringItem = true;

						if ( ImGui::IsMouseDoubleClicked( 0 ) )
						{
							if ( FS::is_directory( p ) )
							{
								mCurrentDirectory = p.path( ).string( ); 
							} 
						} 
					}
				} 
			} 

			// If the list box is hovered
			if ( ImGui::IsMouseHoveringRect( listBoxMin, ImVec2( listBoxMin.x + listBoxSize.x, listBoxMin.y + listBoxSize.y ) ) )
			{
				// Set active context menu and cache mouse coordinates
				if ( !hoveringItem && input->IsKeyPressed( KeyCode::RightMouseButton ) )
				{
					mFolderMenuPopup.Activate( input->GetMouseCoords() );
				} 
			}

			if ( !hoveringItem && input->IsKeyPressed(KeyCode::LeftMouseButton ) )
			{
				SetSelectedPath( "" );
			}

			ImGui::ListBoxFooter( ); 
		} 

		// Do pop-ups
		if ( mFolderMenuPopup.Enabled( ) )
		{
			mFolderMenuPopup.DoWidget( );
		}
	}

	//=========================================================================

	void EditorAssetBrowserView::ProcessViewInput( )
	{ 
	}

	//=========================================================================

	void EditorAssetBrowserView::NewFolderMenuOption( )
	{
		Input* input = EngineSubsystem( Input );
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Folder option group
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImGui::GetStyle( ).Colors[ImGuiCol_TextDisabled] ) );
		ImGui::PushFont( igm->GetFont( "WeblySleek_10" ) );
		{
			ImGui::Text( "Folder" );
		}
		ImGui::PopFont( ); 
		ImGui::PopStyleColor( );

		// Construct new folder option
		if ( ImGui::Selectable( "\t+ New Folder" ) )
		{ 
			// Starting folder name
			String folderName = "NewFolder";
			String dir = mCurrentDirectory + "/NewFolder/";

			// Continue to try and create new directory
			u32 index = 0;
			while ( FS::exists( dir ) )
			{
				index++;
				dir = mCurrentDirectory + "/NewFolder" + std::to_string( index );
			}

			// Attempt to create the directory
			FS::create_directory( dir );

			mFolderMenuPopup.Deactivate( ); 
		} 

		// Separator at end of folder menu option
		ImGui::Separator( );
	}

	//=========================================================================

	void EditorAssetBrowserView::CreateMenuOption( )
	{
		Input* input = EngineSubsystem( Input );
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		AssetManager* am = EngineSubsystem( AssetManager );

		// Folder option group
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImGui::GetStyle( ).Colors[ImGuiCol_TextDisabled] ) );
		ImGui::PushFont( igm->GetFont( "WeblySleek_10" ) );
		{
			ImGui::Text( "Create Assets" );
		}
		ImGui::PopFont( ); 
		ImGui::PopStyleColor( );

		// Construct material option
		if ( ImGui::Selectable( "\t+ Material" ) )
		{ 
			// Construct asset with current directory
			am->ConstructAsset< Material >( "NewMaterial", mCurrentDirectory );

			mFolderMenuPopup.Deactivate( ); 
		} 

		// Separator at end of folder menu option
		ImGui::Separator( );
	}

	void EditorAssetBrowserView::FolderMenuPopup( ) 
	{ 
		Input* input = EngineSubsystem( Input );
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Close folder popup menu
		if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) && !mFolderMenuPopup.Hovered() )
		{
			mFolderMenuPopup.Deactivate( );
		} 

		// New folder option
		NewFolderMenuOption( ); 

		// Create menu option
		CreateMenuOption( );
	}

	void EditorAssetBrowserView::Initialize( )
	{ 
		// Set up folder menu popup
		mFolderMenuPopup.SetSize( Vec2( 200.0f, 400.0f ) );
		mFolderMenuPopup.SetFadeInSpeed( 5.0f ); 
		mFolderMenuPopup.SetFadeOutSpeed( 10.0f );

		// Popup window callback
		mFolderMenuPopup.RegisterCallback( [&]( )
		{ 
			// Do folder menu popup function
			FolderMenuPopup( );
		});
	} 

	//=========================================================================
}

