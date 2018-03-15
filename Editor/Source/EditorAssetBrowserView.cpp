// File: EditorAssetBrowserView.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "EditorAssetBrowserView.h"
#include "EditorApp.h"
#include "Project.h"

#include <Engine.h>
#include <SubsystemCatalog.h>
#include <ImGui/ImGuiManager.h>
#include <IO/InputManager.h>
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

			// If the window is hovered
			if ( ImGui::IsMouseHoveringRect( listBoxMin, ImVec2( listBoxMin.x + listBoxSize.x, listBoxMin.y + listBoxSize.y ) ) )
			{
				// Set active context menu and cache mouse coordinates
				if ( input->IsKeyPressed( KeyCode::RightMouseButton ) )
				{
					mContextMenu.Activate( input->GetMouseCoords() );
				} 
			}

			if ( !hoveringItem && input->IsKeyPressed(KeyCode::LeftMouseButton ) )
			{
				SetSelectedPath( "" );
			}

			ImGui::ListBoxFooter( ); 
		} 

		// Opening context menu
		if ( mContextMenu.IsActive() )
		{
			ContextMenuInteraction( &mContextMenu );
		}
	}

	//=========================================================================

	void EditorAssetBrowserView::ContextMenuInteraction( ContextMenu* menu )
	{ 
		// Get input system
		Input* input = EngineSubsystem( Input ); 

		// Set position of menu
		Vec2 menuPos = menu->GetPosition( ); 
		Vec2 menuSize = menu->GetSize( ); 
		ImVec2 menuMaxPos = ImVec2( menuPos.x + menuSize.x, menuPos.y + menuSize.y );

		// Get display size
		ImVec2 dispSize = ImGui::GetIO( ).DisplaySize; 
 
		// Clamp menu position to inside of window
		if ( menuMaxPos.x > dispSize.x )
		{ 
			menuPos.x = dispSize.x - menuSize.x;
		} 

		// Clamp to being inside of the window and not negative
		menuPos.x = Max( menuPos.x, 0.0f ); 

		// If below half-way ( offset by size of window )
		if ( menuPos.y > dispSize.y / 2.0f )
		{
			menuPos.y -= menuSize.y;
		}

		ImGui::SetNextWindowPos( ImVec2( menuPos.x, menuPos.y ) );
		ImGui::SetNextWindowSize( ImVec2( menuSize.x, menuSize.y ) ); 
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( ImGui::GetStyle( ).Colors[ImGuiCol_PopupBg] ) );
		ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0.0f, 0.0f, 0.0f, 0.8f ) );
		ImGui::Begin( "##assetbrowswercontextmenu", &menu->mOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
		{
			// Close context menu
			if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) && !ImGui::IsMouseHoveringWindow() )
			{
				menu->Deactivate( );
			} 

			// Folder option group
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( ImGui::GetStyle( ).Colors[ImGuiCol_TextDisabled] ) );
			ImGui::PushFont( ImGuiManager::GetFont( "WeblySleek_10" ) );
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

				menu->Deactivate( ); 
			} 

			ImGui::Separator( );
		}
		ImGui::End( );
		ImGui::PopStyleColor( 2 );
	}

	//=========================================================================

	void EditorAssetBrowserView::ProcessViewInput( )
	{ 
	}

	//=========================================================================

	void EditorAssetBrowserView::Initialize( )
	{ 
	}

	//=========================================================================

	void EditorAssetBrowserView::CaptureState( )
	{ 
		Input* input = EngineSubsystem( Input );
		EditorWidgetManager* wm = mApp->GetEditorWidgetManager( );

		ImVec2 wp = ImGui::GetWindowPos( );
		ImVec2 ws = ImGui::GetWindowSize( );

		// Capture hovered state
		bool isHovered = ImGui::IsMouseHoveringRect( wp, ImVec2( wp.x + ws.x, wp.y + ws.y ) );
		wm->SetHovered( this, isHovered );
	}

	//=========================================================================
}

