// Copyright 2016-2019, John Jackson. All Rights Reserved.  
// File: EditorLauncherWindow.cpp

#include "EditorLauncherWindow.h"
#include "EditorApp.h"

#include <Utils/FileUtils.h>
#include <nfd/include/nfd.h> 
#include <fs/filesystem.hpp>

namespace fs = ghc::filesystem; 

namespace Enjon
{
	//=======================================================================================================

	void EditorLauncherWindow::Init( const WindowParams& params )
	{ 
		// Set app from data
		mApp = ( EditorApp* )( params.mData );
		mNewProject.SetProjectPath( mApp->GetConfigSettings()->mLastUsedProjectDirectory );
		mNewProject.SetProjectName( "NewProject" );

		ConstructGUI( ); 
	}

	//=======================================================================================================

	bool EditorLauncherWindow::ProjectSelectionBox( const Project& p )
	{ 
		static const Project* selectedProject = nullptr;
		bool retVal = false;

		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		ImVec2 a = ImVec2(ImGui::GetCursorScreenPos().x - 2.f, ImGui::GetCursorScreenPos().y - 2.f);
		ImVec2 b = ImVec2(a.x + ImGui::GetWindowWidth() * 0.93f, a.y + 35.f);
		b32 hovered = ImGui::IsMouseHoveringRect(a, b);
		b32 active = hovered && ImGui::IsMouseClicked(0);
		b32 selected = selectedProject == &p;
		ImColor color = selected ? ImGui::GetColorU32(ImGuiCol_HeaderActive) : 
						hovered ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : 
						ImColor(0.f, 0.f, 0.f, 0.f);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRect(ImVec2( a.x, a.y ), b, color, 1.2f);
		ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 2.f );
		ImGui::Text("%s", p.GetProjectName().c_str()); 
		if ( selected ) {
			String lab = "Launch";
			f32 margin = 10.f;
			f32 pad = 2.f;
			ImVec2 ts = ImGui::CalcTextSize(lab.c_str());
			ImVec2 ba = ImVec2(b.x - ts.x - margin - pad, (b.y + a.y - ts.y) / 2.f - pad);
			ImVec2 bb = ImVec2(ba.x + ts.x + 2.f * pad, ba.y + ts.y + 2.f * pad);
			ImVec2 ta = ImVec2(b.x - ts.x - margin, (b.y + a.y - ts.y) / 2.f);
			b32 bHovered = ImGui::IsMouseHoveringRect(ba, bb);
			b32 bActive = bHovered && ImGui::IsMouseClicked(0);
			ImColor bColor = bActive ? ImColor(1.f, 1.f, 1.f, 1.f) :
							bHovered ? ImColor(1.f, 1.f, 1.f, 0.7f) :
							ImColor(1.f, 1.f, 1.f, 0.4f);
			dl->AddText(igm->GetFont("Roboto-MediumItalic_14"), 14, ta, bColor, lab.c_str());
			
			if ( bActive ) 
			{ 
				retVal = true;
			}
		}
		
		// Doing the path... need to calculate the size to make sure the path isn't too large past the window
		{
			u32 len = 38;
			bool gtl = p.GetProjectPath().length() >= len;
			String b = p.GetProjectPath().substr( 0, gtl ? len : p.GetProjectPath().length() );
			b += gtl ? "..." : "";
			ImGui::PushFont(igm->GetFont("Roboto-MediumItalic_12")); 
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 2.f );
			ImGui::Text("%s", b.c_str());
			ImGui::PopFont();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.f);
			if ( active ) {
				selectedProject = &p;	// Pretty unsafe, but oh well...
			} 
		}

		return retVal;

	}

	//=======================================================================================================

	void EditorLauncherWindow::ProjectSelectionScreen()
	{
		// Let's list all the available projects for now

		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		ImGui::SetNextWindowPos( ImVec2( 0.f, 0.f ) );
		ImGui::SetNextWindowSize( ImVec2( ImGui::GetWindowWidth() * 0.35f, ImGui::GetWindowHeight() ) );
		ImGui::Begin( "##project_list", nullptr, ImGuiWindowFlags_NoCollapse | 
						ImGuiWindowFlags_NoResize | 
						ImGuiWindowFlags_NoTitleBar | 
						ImGuiWindowFlags_NoScrollbar | 
						ImGuiWindowFlags_NoScrollWithMouse 
		);
		{
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 bga = ImVec2( -5.f, 0.f );
			ImVec2 bgb = ImVec2( bga.x + ImGui::GetWindowWidth() + 5.f, bga.y + ImGui::GetWindowHeight() );
			ImColor col = ImGui::GetColorU32( ImGuiCol_FrameBg );
			col.Value.w = 1.f;
			dl->AddRectFilled( bga, bgb, col );
			ImVec2 ws = ImVec2( ImGui::GetWindowWidth(), ImGui::GetWindowHeight() );
			ImGui::SetCursorPosY( 0.f );
			ImGui::ListBoxHeader( "##available_project_list", ws );
			{
				EditorConfigSettings* cfg = mApp->GetConfigSettings();

				for ( auto& p : cfg->mProjectList )
				{
					if ( ProjectSelectionBox( p ) )
					{ 
						mApp->PreloadProject( p );
						WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
						ws->DestroyWindow( GetWindowID() );
					}
				}
			}
			ImGui::ListBoxFooter(); 
		}
		ImGui::End(); 
	}

	//=======================================================================================================

	void EditorLauncherWindow::LoadProjectScreen()
	{ 
		nfdchar_t* outPath = NULL;
		nfdresult_t res = NFD_PickFolder( NULL, &outPath );
		if ( res == NFD_OKAY )
		{
			// Set the path now
			// Need to check that the directory contains a .eproj file to be considered valid

			if ( fs::exists( outPath ) && fs::is_directory( outPath ) )
			{
				for ( auto& p : ghc::filesystem::recursive_directory_iterator( outPath ) )
				{
					// Get file extension of passed in file
					String fileExt = "." + Utils::SplitString( p.path().string(), "." ).back( );
					if ( fileExt.compare( ".eproj" ) == 0 )
					{
						// Load project (want to get all of this information from the project directory, obviously)
						Project proj; 
						proj.SetProjectPath( String( outPath ) + "/" );
						proj.SetProjectName( "NewProject" );					// This isn't correct...
						proj.SetEditor( mApp );
						mApp->PreloadProject( proj );
						mApp->GetConfigSettings()->mProjectList.push_back(proj);
						WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
						ws->DestroyWindow( GetWindowID() );

						// Serialize editor settings after loading project
						mApp->SerializeEditorConfigSettings();
					}
				}
			} 
		}
	}

	//=======================================================================================================

	void EditorLauncherWindow::ProjectMenuScreen()
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		ImGui::SetNextWindowPos( ImVec2( ImGui::GetWindowWidth() * 0.35f, 0.f ) );
		ImGui::SetNextWindowSize( ImVec2( ImGui::GetWindowWidth() * 0.65f, ImGui::GetWindowHeight() ) );
		ImGui::Begin( "##project_crud", nullptr, ImGuiWindowFlags_NoCollapse | 
						ImGuiWindowFlags_NoResize | 
						ImGuiWindowFlags_NoTitleBar | 
						ImGuiWindowFlags_NoScrollbar | 
						ImGuiWindowFlags_NoScrollWithMouse 
		);
		{
			// Title
			ImGui::SetCursorPosY( ImGui::GetWindowHeight() / 5.f );
			{
				igm->PushFont( "WeblySleek_32" );
				String labl = "Enjon Editor";
				ImVec2 ts = ImGui::CalcTextSize( labl.c_str() );
				ImGui::SetCursorPosX( ( ImGui::GetWindowWidth() - ts.x ) / 2.f );
				igm->Text( labl.c_str() );
				igm->PopFont(); 
			}

			{
				igm->PushFont( "WeblySleek_14" );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, 0.4f) );
				String labl = "Version 0.01";
				ImVec2 ts = ImGui::CalcTextSize( labl.c_str() );
				ImGui::SetCursorPosX( ( ImGui::GetWindowWidth() - ts.x ) / 2.f );
				igm->Text( labl.c_str() );
				ImGui::PopStyleColor();
				igm->PopFont(); 
			}

			ImGui::NewLine();

			ImDrawList* dl = ImGui::GetWindowDrawList();

			// Create new project 
			{
				const char* labl = "+ Create New Enjon Project";
				ImVec2 ts = ImGui::CalcTextSize( labl );
				ImVec2 a = ImVec2( ImGui::GetCursorScreenPos().x + ( ImGui::GetWindowWidth() - ts.x ) / 2.f - 10.f, ImGui::GetCursorScreenPos().y );
				ImVec2 b = ImVec2( a.x + ts.x, a.y + ts.y );
				bool hovered = ImGui::IsMouseHoveringRect( a, b );
				bool active = hovered && ImGui::IsMouseDown( 0 );
				ImColor col = active ? ImGui::GetColorU32( ImGuiCol_ListSelectionActive ) :
								hovered ? ImGui::GetColorU32( ImGuiCol_ListSelectionHovered )  :
								ImGui::GetColorU32( ImGuiCol_Text );
				dl->AddText( igm->GetFont("Roboto-MediumItalic_14"), 14, a, col, labl );

				if ( active ) 
				{
					mScreenType = EditorLauncherScreenType::ProjectCreation;
				}
			}
			ImGui::NewLine();

			// Load existing project 
			{
				const char* labl = "Load Existing Enjon Project";
				ImVec2 ts = ImGui::CalcTextSize( labl );
				ImVec2 a = ImVec2( ImGui::GetCursorScreenPos().x + ( ImGui::GetWindowWidth() - ts.x ) / 2.f - 10.f, ImGui::GetCursorScreenPos().y );
				ImVec2 b = ImVec2( a.x + ts.x, a.y + ts.y );
				bool hovered = ImGui::IsMouseHoveringRect( a, b );
				bool active = hovered && ImGui::IsMouseDown( 0 );
				ImColor col = active ? ImGui::GetColorU32( ImGuiCol_ListSelectionActive ) :
								hovered ? ImGui::GetColorU32( ImGuiCol_ListSelectionHovered )  :
								ImGui::GetColorU32( ImGuiCol_Text );
				dl->AddText( igm->GetFont("Roboto-MediumItalic_14"), 14, a, col, labl );

				if ( active )
				{
					LoadProjectScreen();
				}
			}
		}
		ImGui::End(); 
	}

	//=======================================================================================================
	
	void EditorLauncherWindow::ProjectCreationScreen()
	{
			// Want to create a view for listing all available projects.
			ImGui::SetNextWindowPos( ImVec2( 0.f, 0.f ) );
			ImGui::SetNextWindowSize( ImGui::GetIO().DisplaySize );
			ImGui::Begin( "##create_project", nullptr, ImGuiWindowFlags_NoCollapse | 
							ImGuiWindowFlags_NoResize | 
							ImGuiWindowFlags_NoTitleBar | 
							ImGuiWindowFlags_NoScrollbar | 
							ImGuiWindowFlags_NoScrollWithMouse 
			);
			{ 
				ImGuiManager* igm = EngineSubsystem( ImGuiManager );

				ImGui::NewLine();
				ImGui::NewLine();
				igm->PushFont( "WeblySleek_24" );
				String labl = "Create Project";
				igm->Text( labl.c_str() );
				igm->PopFont(); 

				ImGui::SetCursorPosY( ImGui::GetWindowHeight() * 0.4f );

				{
					char tmpBuffer[1024];
					String pName = mNewProject.GetProjectName();
					String pDir = mNewProject.GetProjectPath();
					strncpy( tmpBuffer, pName.c_str(), 1024 );
					char buffer[2048];
					snprintf( (char*)buffer, 2048, "%s/%s", pDir.c_str(), tmpBuffer );
					b32 projExists = fs::exists( buffer );
					b32 dirExists = fs::exists( pDir );
					b32 projVal = dirExists && !projExists;
					ImGui::PushItemWidth( ImGui::GetWindowWidth() * 0.8f );
					ImGui::PushStyleColor( ImGuiCol_Text, projVal ? ImGui::GetColorU32( ImGuiCol_Text ) : ImColor( 0.9f, 0.1f, 0.f, 1.f ) );
					ImGui::Text( "Name" ); ImGui::SameLine(); ImGui::SetCursorPosX( 100.f );
					if (ImGui::InputText( "##proj_name", tmpBuffer, 150 ))
					{
						snprintf( (char*)buffer, 2048, "%s/%s", pDir.c_str(), tmpBuffer );
						if (fs::exists( buffer ))
						{
							projExists = true;
						}
						mNewProject.SetProjectName( tmpBuffer );
					}
					ImGui::PopStyleColor();
					ImGui::PopItemWidth(); 
				} 

				{
					char tmpBuffer[1024];
					String pName = mNewProject.GetProjectName();
					String pDir = mNewProject.GetProjectPath();
					strncpy( tmpBuffer, pDir.c_str(), 1024 );
					b32 dirExists = fs::exists( tmpBuffer );
					ImGui::PushItemWidth( ImGui::GetWindowWidth() * 0.77f );
					ImGui::PushStyleColor( ImGuiCol_Text, dirExists ? ImGui::GetColorU32( ImGuiCol_Text ) : ImColor( 0.9f, 0.1f, 0.f, 1.f ) );
					ImGui::Text( "Location" ); ImGui::SameLine(); ImGui::SetCursorPosX( 100.f );
					if (ImGui::InputText( "##proj_dir", tmpBuffer, 250 ))
					{
						mNewProject.SetProjectPath( tmpBuffer ); 
					}
					ImGui::PopItemWidth(); 
					ImGui::PopStyleColor();

					// Do button for selecting directory
					ImGui::SameLine(); ImGui::SetCursorPosX( ImGui::GetWindowWidth() * 0.889f );
					if ( ImGui::Button( "..." ) ) 
					{
						// Open file picking dialogue
						nfdchar_t* folder;
						nfdresult_t res = NFD_PickFolder( NULL, &folder );
						if (res == NFD_OKAY)
						{
							mNewProject.SetProjectPath( folder );
							mApp->GetConfigSettings()->mLastUsedProjectDirectory = String( folder );
							mApp->SerializeEditorConfigSettings();
						}
					}
				} 

				{ 
					EditorConfigSettings* cfgSettings = mApp->GetConfigSettings();
					String defaultText = mToolChainDef == nullptr ? "Tool Chains..." : mToolChainDef->mLabel;
					b32 tcVal = mToolChainDef != nullptr;

					ImGui::PushStyleColor( ImGuiCol_Text, tcVal ? ImGui::GetColorU32( ImGuiCol_Text ) : ImColor( 0.9f, 0.1f, 0.f, 1.f ) );
					ImGui::Text( "Tool Chain" ); ImGui::SameLine(); ImGui::SetCursorPosX( 100.f );
					ImGui::PopStyleColor();
					ImGui::PushItemWidth( ImGui::GetWindowWidth() * 0.8f ); 
					if (ImGui::BeginCombo( "##TOOL_CHAINS", defaultText.c_str() ))
					{
						for (u32 i = 0; i < cfgSettings->mToolChainDefinitions.size(); ++i)
						{
							ToolChainDefinition* tc = &cfgSettings->mToolChainDefinitions[i];
							if (ImGui::Selectable( tc->mLabel.c_str() ))
							{
								mToolChainDef = tc;
							}
						}
						ImGui::EndCombo();
					}
				}

				{
					ImGui::SetCursorPosX( 100.f );
					if ( ImGui::Button( "Cancel" ) )
					{
						mScreenType = EditorLauncherScreenType::Main;
					} 
				}

				ImGui::SameLine();

				b32 dirVal = fs::exists( mNewProject.GetProjectPath() );
				b32 projVal = dirVal && !fs::exists( mNewProject.GetProjectPath() + "/" + mNewProject.GetProjectName() );
				b32 tcVal = mToolChainDef != nullptr;
				b32 allVal = dirVal && projVal && tcVal;
				{
					ImGui::PushStyleColor( ImGuiCol_Button, projVal ? ImGui::GetColorU32( ImGuiCol_Button ) : ImColor(0.3f, 0.3f, 0.3f, 0.2f) );
					ImGui::PushStyleColor( ImGuiCol_ButtonActive, projVal ? ImGui::GetColorU32( ImGuiCol_ButtonActive) : ImColor(0.3f, 0.3f, 0.3f, 0.2f) );
					ImGui::PushStyleColor( ImGuiCol_ButtonHovered, projVal ? ImGui::GetColorU32( ImGuiCol_ButtonHovered ) : ImColor(0.3f, 0.3f, 0.3f, 0.2f) );
					ImGui::PushStyleColor( ImGuiCol_Text, projVal ? ImGui::GetColorU32( ImGuiCol_Text ) : ImColor(1.f, 1.f, 1.f, 0.2f) );
					if ( ImGui::Button( "Create Project" ) )
					{
						if ( allVal )
						{
							// Do things...
							ProjectConfig config;
							config.mPath = mNewProject.GetProjectPath() + "/" + mNewProject.GetProjectName() + "/";
							config.mName = mNewProject.GetProjectName();
							config.mToolChain = *mToolChainDef; 
							mApp->PreCreateNewProject( config ); 
							WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
							ws->DestroyWindow( GetWindowID() );
						}
					} 
					ImGui::PopStyleColor(4);
				}

				if ( !dirVal )
				{ 
					ImGui::SetCursorPosX( 100.f );
					ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.9f, 0.1f, 0.f, 1.f ) );
					ImGui::Text( "Please choose a valid directory." );
					ImGui::PopStyleColor();
				} 
				else if ( !projVal )
				{
					ImGui::SetCursorPosX( 100.f );
					ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.9f, 0.1f, 0.f, 1.f ) );
					if (mNewProject.GetProjectName().empty()) {
						ImGui::Text( "Project name cannot be empty." ); 
					}
					else {
						ImGui::Text( "Project already exists. Please choose a different project name." ); 
					}
					ImGui::PopStyleColor(); 
				}
				if ( !tcVal )
				{
					ImGui::SetCursorPosX( 100.f );
					ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.9f, 0.1f, 0.f, 1.f ) );
					ImGui::Text( "Tool Chain not set. Please select a valid tool chain." );
					ImGui::PopStyleColor(); 
				}
			}
			ImGui::End(); 
	}

	//=======================================================================================================

	void EditorLauncherWindow::ConstructGUI()
	{ 
		GUIContext* guiContext = GetGUIContext( ); 

		guiContext->RegisterMainMenu( "File" );

		auto stylesMenuOption = [&]()
		{ 
			if ( ImGui::MenuItem( "Styles##options" ) )
			{
				ImGui::ShowStyleEditor();	
			}
		};

		guiContext->RegisterMenuOption("File", "Styles##Options", stylesMenuOption);

		guiContext->RegisterWindow( "Project Browser", [&]
		{ 
			// Want to create a view for listing all available projects.
			ImGui::SetNextWindowPos( ImVec2( 0.f, 0.f ) );
			ImGui::SetNextWindowSize( ImGui::GetIO().DisplaySize );
			ImGui::Begin( "##ProjectList", nullptr, ImGuiWindowFlags_NoCollapse | 
							ImGuiWindowFlags_NoResize | 
							ImGuiWindowFlags_NoTitleBar | 
							ImGuiWindowFlags_NoScrollbar | 
							ImGuiWindowFlags_NoScrollWithMouse 
			);
			{ 
				switch ( mScreenType )
				{
					case EditorLauncherScreenType::Main:
					{
						// List all available projects on one side of the screen ( first column )
						// Want to have access to editor app's things
						// On second column, list project creation screen ( Create New Project, Load Project, etc. ) 
						ProjectSelectionScreen(); 

						// Second column, list actions for creating/loading projects
						ProjectMenuScreen(); 
					} break;

					case EditorLauncherScreenType::ProjectCreation: 
					{
						ProjectCreationScreen(); 
					} break;
				} 
			}
			ImGui::End(); 
		});

		GUIContextParams params = { false };
		guiContext->SetGUIContextParams( params );
		guiContext->Finalize( );
	}

	//======================================================================================================= 
} 
