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

	void EditorAssetBrowserView::PushActivePopupWindow( PopupWindow* window )
	{
		mActivePopupWindow = window;
	}

	//=========================================================================

	bool EditorAssetBrowserView::ActivePopupWindowEnabled( )
	{
		if ( mActivePopupWindow == nullptr )
		{
			return false;
		}

		return mActivePopupWindow->Enabled( ); 
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

			// Add new button
			if ( ImGui::Button( "+ Add New" ) )
			{
				mFolderMenuPopup.Activate( input->GetMouseCoords( ) );
			} 

			ImGui::SameLine( );

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
			ImVec2 listBoxSize = ImVec2( ImGui::GetWindowWidth( ) - 20.0f, ImGui::GetWindowHeight( ) - 100.0f );
			bool hoveringItem = false;
			ImGui::ListBoxHeader( "##AssetDirectory", listBoxSize );
			{
				for ( auto& p : FS::directory_iterator( mCurrentDirectory ) )
				{
					bool isDir = FS::is_directory( p );
					bool isSelected = p == FS::path( mSelectedPath );

					String pathLabel = Utils::SplitString( p.path( ).string( ), "\\" ).back( );
					ImColor headerHovered = ImColor( ImGui::GetColorU32( ImGuiCol_HeaderHovered ) );
					ImColor textColor = ImColor( ImGui::GetColorU32( ImGuiCol_Text ) );

					// Get file extension
					String fileExtension = isDir ? "" : Utils::SplitString( pathLabel, "." ).back( );

					// Select directory
					ImColor finalColor = isDir ? isSelected ? textColor : headerHovered : textColor ;

					ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( finalColor ) );
					if ( mPathNeedsRename && p == FS::path( mSelectedPath ) )
					{
						ImGui::SetKeyboardFocusHere( -1 );

						char buffer[ 256 ];
						strncpy( buffer, pathLabel.c_str( ), 256 );
						if ( ImGui::InputText( "##pathRename", buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll ) )
						{
							pathLabel = buffer;

							String newPath = "";
							Vector<String> splits = Utils::SplitString( p.path( ).string( ), "\\" );
							for ( u32 i = 0; i < splits.size( ) - 1; ++i )
							{
								newPath += splits.at( i );
								newPath += i == splits.size( ) - 2 ? "" : "/";
							}

							// Rename the path if it doens't exist
							String finalPath = newPath + "/" + pathLabel + fileExtension;
							if ( !FS::exists( finalPath ) )
							{
								FS::rename( p, newPath + "/" + pathLabel + fileExtension ); 
							}

							mPathNeedsRename = false;
						}

						if ( input->IsKeyPressed( KeyCode::LeftMouseButton ) && !ImGui::IsItemHovered( ) )
						{
							mPathNeedsRename = false;
						}
					} 
					else if ( ImGui::Selectable( pathLabel.c_str( ), isSelected ) )
					{
						SetSelectedPath( p.path( ).string( ) );
					}
					ImGui::PopStyleColor( );

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

						if ( input->IsKeyPressed( KeyCode::RightMouseButton ) )
						{
							SetSelectedPath( p.path( ).string( ) );
						}
					}
				} 
			} 

			// If the list box is hovered
			if ( ImGui::IsMouseHoveringRect( listBoxMin, ImVec2( listBoxMin.x + listBoxSize.x, listBoxMin.y + listBoxSize.y ) ) )
			{
				// Set active context menu and cache mouse coordinates
				if ( input->IsKeyPressed( KeyCode::RightMouseButton ) )
				{
					if ( !hoveringItem )
					{
						mFolderMenuPopup.Activate( input->GetMouseCoords( ) );
						PushActivePopupWindow( &mFolderMenuPopup );
					}
					else
					{
						if ( FS::is_directory( mSelectedPath ) )
						{
							mFolderOptionsMenuPopup.Activate( input->GetMouseCoords( ) ); 
							PushActivePopupWindow( &mFolderOptionsMenuPopup );
						} 
					}
				} 
			}

			// TODO(): Input states are starting to get wonky - need to unify through one central area
			if ( !hoveringItem && !mPathNeedsRename && !ActivePopupWindowEnabled() && ( input->IsKeyPressed(KeyCode::LeftMouseButton ) || input->IsKeyPressed( KeyCode::RightMouseButton ) ) )
			{
				SetSelectedPath( "" );
			}

			ImGui::ListBoxFooter( ); 
		} 

		// Do active popup window
		if ( ActivePopupWindowEnabled( ) )
		{
			mActivePopupWindow->DoWidget( );
		} 

		if ( mSelectedPath.compare( "" ) != 0 && input->IsKeyPressed( KeyCode::F2 ) && FS::is_directory( mSelectedPath ) )
		{
			mPathNeedsRename = true;
		}
	}

	//=========================================================================

	void EditorAssetBrowserView::ProcessViewInput( )
	{ 
	}

	//=========================================================================

	void EditorAssetBrowserView::FolderOptionsMenuPopup( )
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
		if ( ImGui::Selectable( "\tDelete" ) )
		{ 
			// Attempt to create the directory
			FS::remove_all( mSelectedPath );

			mFolderOptionsMenuPopup.Deactivate( ); 
		} 

		// Construct new folder option
		if ( ImGui::Selectable( "\tRename" ) )
		{ 
			mPathNeedsRename = true; 

			mFolderOptionsMenuPopup.Deactivate( ); 
		} 

		if ( input->IsKeyDown( KeyCode::LeftMouseButton ) && !mFolderOptionsMenuPopup.Hovered( ) )
		{
			mFolderOptionsMenuPopup.Deactivate( );
		}

		// Separator at end of folder menu option
		ImGui::Separator( ); 
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
			String dir = mCurrentDirectory + "\\NewFolder";

			// Continue to try and create new directory
			u32 index = 0;
			while ( FS::exists( dir ) )
			{
				index++;
				dir = mCurrentDirectory + "\\NewFolder" + std::to_string( index );
			}

			// Attempt to create the directory
			FS::create_directory( dir );

			// Select the path
			SetSelectedPath( dir );

			// Set to needing rename
			mPathNeedsRename = true; 

			mFolderMenuPopup.Deactivate( ); 
		} 

		if ( input->IsKeyDown( KeyCode::LeftMouseButton ) && !mFolderMenuPopup.Hovered( ) )
		{
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
		mFolderMenuPopup.SetSize( Vec2( 200.0f, 400.0f ) );
		mFolderOptionsMenuPopup.SetSize( Vec2( 200.0f, 400.0f ) );

		// Set up folder menu popup 
		mFolderMenuPopup.RegisterCallback( [&]( )
		{ 
			// Do folder menu popup function
			FolderMenuPopup( );
		} );

		// Set up folder options menu popup
		mFolderOptionsMenuPopup.RegisterCallback( [ & ] ( )
		{
			FolderOptionsMenuPopup( );
		} );
	} 

	//=========================================================================
}

