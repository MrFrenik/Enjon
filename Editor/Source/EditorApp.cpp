// @file EditorApp.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.  

#include "EditorApp.h"
#include "EditorSceneView.h"
#include "EditorAssetBrowserView.h"
#include "EditorInspectorView.h" 
#include "EditorWorldOutlinerView.h"
#include "EditorMaterialEditWindow.h"

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <Serialize/ObjectArchiver.h>
#include <Serialize/EntityArchiver.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>
#include <ImGui/ImGuiManager.h>
#include <Graphics/GraphicsSubsystem.h> 
#include <Graphics/Window.h>
#include <Entity/EntityManager.h>
#include <Physics/PhysicsSubsystem.h>
#include <Scene/SceneManager.h>
#include <Entity/Components/StaticMeshComponent.h>
#include <Entity/Components/RigidBodyComponent.h>
#include <Entity/Components/PointLightComponent.h>
#include <Entity/Components/DirectionalLightComponent.h>
#include <Entity/Components/CameraComponent.h>
#include <Utils/FileUtils.h> 
#include <Utils/Tokenizer.h> 

#include <Base/World.h> 

#include <windows.h>
#include <fmt/format.h>
#include <chrono>
#include <ctime>
#include <nfd/include/nfd.h>

/*
	- This define is funky. Delete all the engine assets in the engine asset cache. Set this define to 1. Load the editor to 
		recreate all engine assets. STOP the editor. Set this define back to 0. Then you're go to run like normal. 
*/
#define LOAD_ENGINE_RESOURCES	0

typedef void( *funcSetEngineInstance )( Enjon::Engine* instance );
typedef Enjon::Application*( *funcCreateApp )( Enjon::Engine* );
typedef void( *funcDeleteApp )( Enjon::Application* ); 

// TODO(): Make sure to abstract this for platform independence
HINSTANCE dllHandleTemp = nullptr;
HINSTANCE dllHandle = nullptr;
funcSetEngineInstance setEngineFunc = nullptr;
funcCreateApp createAppFunc = nullptr;
funcDeleteApp deleteAppFunc = nullptr;

namespace fs = std::experimental::filesystem; 

// ALL of this needs to change to be user dependent
Enjon::String projectName = "TestProject";
Enjon::String projectDLLName = projectName + ".dll";
Enjon::String copyDir = ""; 
//Enjon::String mProjectsDir = "E:/Development/EnjonProjects/";
Enjon::String mProjectsDir = "";
//Enjon::String mVisualStudioDir = "\"E:\\Programs\\MicrosoftVisualStudio14.0\\\"";
Enjon::String mVisualStudioDir = ""; 

// Need to make two different builds for editor configurations - debug / release

// Should have release/debug builds of the engine and editors for people. That way they can load different versions and debug if necessary.

// Need to come up with a way to set the current configuration for the editor - or at the very least, just replace the current .dll with what is loaded? 
//Enjon::String configuration = "Release";
//Enjon::String configuration = "RelWithDebInfo";
Enjon::String configuration = "Debug";
//Enjon::String configuration = "Bin";

namespace Enjon
{
	// This has to happen anyway. Perfect!
	void CopyLibraryContents( const String& projectName, const String& projectDir )
	{
		Enjon::String rootDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( );
		Enjon::String dllName =  projectName + ".dll";

		fs::path dllPath = rootDir + "Build/" + configuration + "/" + dllName;
		if ( fs::exists( dllPath ) )
		{
			fs::remove( dllPath );
		}

		// Now copy over contents from intermediate build to executable dir
		dllPath = projectDir;
		if ( fs::exists( dllPath ) )
		{
			if ( fs::exists( dllPath.string( ) + "Build/" + configuration + "/" + dllName ) )
			{
				fs::copy( fs::path( dllPath.string( ) + "Build/" + configuration + "/" + dllName ), rootDir + "Build/" + configuration + "/" + dllName );
			}
		}
	}

	void EditorApp::LoadProjectView( )
	{ 
		if ( !mPlaying )
		{ 
			const char* popupName = "Load Project##Modal";
			if ( !ImGui::IsPopupOpen( popupName ) )
			{
				ImGui::OpenPopup( popupName ); 
			}
			ImGui::SetNextWindowSize( ImVec2( 600.0f, 150.0f ) );
			if( ImGui::BeginPopupModal( popupName, NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove ) )
			{
					String defaultText = mProject.GetApplication( ) == nullptr ? "Existing Projects..." : mProject.GetProjectName( );
					if ( ImGui::BeginCombo( "##LOADPROJECTLIST", defaultText.c_str() ) )
					{
						for ( auto& p : mProjectsOnDisk )
						{
							if ( ImGui::Selectable( p.GetProjectName( ).c_str( ) ) )
							{ 
								// Load the project
								LoadProject( p );
								mLoadProjectPopupDialogue = false;
								ImGui::CloseCurrentPopup( );
							}
						}

						ImGui::EndCombo( );
					} 

					if ( ImGui::Button( "Cancel" ) )
					{
						mLoadProjectPopupDialogue = false;
						ImGui::CloseCurrentPopup( );

					}

				ImGui::EndPopup( );
			} 
		} 
		else
		{
			mLoadProjectPopupDialogue = false;
		}
	}

	void EditorApp::AddComponentPopupView( )
	{ 
		ImGui::SetNextWindowSize( ImVec2( 500.0f, 120.0f ) );

		if ( ImGui::BeginPopupModal( "Add C++ Component##NewComponent", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
		{ 
			// Get component list
			EntityManager* entities = EngineSubsystem( EntityManager );
			auto compMetaClsList = entities->GetComponentMetaClassList( );

			static String componentErrorMessage = "";
			static String componentName;
			bool closePopup = false;

			// Use to check if the component already exists
			auto alreadyExists = [ & ] ( const String& name ) -> bool
			{
				for ( auto& c : compMetaClsList )
				{
					if ( c->GetName( ).compare( name ) == 0 )
					{
						return true;
					} 
				}

				return false;
			};

			char buffer[ 256 ];
			strncpy( buffer, componentName.c_str( ), 256 );
			if ( ImGui::InputText( "Component Name", buffer, 256, ImGuiInputTextFlags_AlwaysInsertMode | ImGuiInputTextFlags_AllowTabInput ) )
			{
				// Reset component name
				componentName = String(buffer);
			} 

			bool compExists = true;
			bool isValidCPPClassName = true;

			if ( ImGui::Button( "Create" ) )
			{
				compExists = alreadyExists( componentName );
				isValidCPPClassName = Utils::IsValidCPPClassName( componentName );

				if ( !compExists && isValidCPPClassName )
				{ 
					// Get total count of entities
					UUID curUUID = mWorldOutlinerView->GetSelectedEntity( ).Get( )->GetUUID( );

					std::cout << "Creating component!\n";
					closePopup = true;
					CreateComponent( componentName ); 

					// Force add all the entities ( NOTE(): HATE THIS )
					EntityManager* em = EngineSubsystem( EntityManager );
					em->ForceAddEntities( );

					// Reselect entity by uuid
					mWorldOutlinerView->SelectEntity( em->GetEntityByUUID( curUUID ) );
					//SelectEntity( em->GetEntityByUUID( curUUID ) );

					// After creating new component, need to attach to entity
					if ( mWorldOutlinerView->GetSelectedEntity() )
					{
						// Add using meta class 
						mWorldOutlinerView->GetSelectedEntity( ).Get( )->AddComponent( Object::GetClass( componentName ) );
					}

					// Attempt to open the header in visual studio
					String headerFilePath = mProject.GetProjectPath( ) + "Source/" + componentName + ".h"; 
					s32 code = system( String( "start " + headerFilePath ).c_str() ); 

				}
				else if ( compExists )
				{ 
					componentErrorMessage = "Component already exists!";
				}
				else
				{
					componentErrorMessage = "Not a valid C++ class name!";
				}
			}

			ImGui::SameLine( );

			if ( ImGui::Button( "Cancel" ) )
			{
				closePopup = true;
			}

			if ( closePopup )
			{
				ImGui::CloseCurrentPopup( );
				mNewComponentPopupDialogue = false; 
				componentErrorMessage = "";
				componentName = "";
			}

			// If not empty string
			if ( componentErrorMessage.compare( "" ) != 0 )
			{
				ImGui::Text( componentErrorMessage.c_str() );
			}

			ImGui::EndPopup( );
		}
	}

	//==================================================================================================================

	void EditorApp::CreateComponent( const String& componentName )
	{ 
		// Replace meta tags with component name
		String includeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", mComponentSourceTemplate, false ), "#COMPONENTNAME", componentName );
		String sourceFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", mComponentSourceTemplate, false ), "#COMPONENTNAME", componentName ); 

		// Need to copy the include files into the source directory for the project
		Utils::WriteToFile( includeFile, mProject.GetProjectPath( ) + "Source/" + componentName + ".h" );
		Utils::WriteToFile( sourceFile, mProject.GetProjectPath( ) + "Source/" + componentName + ".cpp" );

		String cmakePath = mProject.GetProjectPath( ) + "CMakeLists.txt";
		if ( fs::exists( cmakePath ) )
		{
			// Grab the content
			String cmakeContent = Utils::read_file_sstream( cmakePath.c_str() ); 
			// Resave it 
			Utils::WriteToFile( cmakeContent, cmakePath );
		} 

		// Compile the project
		mProject.CompileProject( );

		// Save scene (this seems to be acting strangely)
		SceneManager* sm = EngineSubsystem( SceneManager );
		if ( sm->GetScene() )
		{
			sm->GetScene()->Save( );
		}

		ReloadDLL( );
	}

	//==================================================================================================================

	EntityHandle EditorApp::GetSelectedEntity( )
	{
		return mWorldOutlinerView->GetSelectedEntity();
	}

	//==================================================================================================================

	EditorInspectorView* EditorApp::GetInspectorView( )
	{
		return mInspectorView;
	}

	//==================================================================================================================

	EditorAssetBrowserView* EditorApp::GetEditorAssetBrowserView( )
	{
		return mAssetBroswerView;
	} 

	//==================================================================================================================

	void EditorApp::OpenNewComponentDialogue( )
	{
		if ( mNewComponentPopupDialogue )
		{
			ImGui::OpenPopup( "Add C++ Component##NewComponent" );
			AddComponentPopupView( ); 
		}
	}
	
	//==================================================================================================================

	void EditorApp::EnableOpenNewComponentDialogue( )
	{
		mNewComponentPopupDialogue = mNewComponentPopupDialogue = true;; 
	}

	//==================================================================================================================

	void EditorApp::SceneView( bool* viewBool )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		u32 currentTextureId = gfx->GetCurrentRenderTextureId( ); 

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		// Cache off cursor position for scene view
		mSceneViewWindowPosition = Vec2( cursorPos.x, cursorPos.y );
		mSceneViewWindowSize = Vec2( ImGui::GetWindowWidth( ), ImGui::GetWindowHeight( ) );

		ImTextureID img = ( ImTextureID )currentTextureId;
		ImGui::Image( img, ImVec2( ImGui::GetWindowWidth( ), ImGui::GetWindowHeight( ) ),
			ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		ImVec2 min = ImVec2( cursorPos.x + ImGui::GetContentRegionAvailWidth( ) - 100.0f, cursorPos.y + 10.0f );
		ImVec2 max = ImVec2( min.x + 50.0f, min.y + 10.0f );

		ImGui::SetCursorScreenPos( min );
		auto drawlist = ImGui::GetWindowDrawList( );
		f32 fps = ImGui::GetIO( ).Framerate;

		// Update camera aspect ratio
		gfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );
	}

	Vec2 EditorApp::GetSceneViewProjectedCursorPosition( )
	{
		return mEditorSceneView->GetSceneViewProjectedCursorPosition( );
	}

	void EditorApp::CameraOptions( bool* enable )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		const Enjon::Camera* cam = gfx->GetGraphicsSceneCamera( );

		if ( ImGui::TreeNode( "Camera" ) )
		{
			igm->DebugDumpObject( cam ); 
			ImGui::TreePop( );
		}

		ImGui::DragFloat( "Camera Speed", &mCameraSpeed, 0.01f, 0.01f, 100.0f ); 
		ImGui::DragFloat( "Mouse Sensitivity", &mMouseSensitivity, 0.1f, 0.1f, 10.0f ); 
		f32 col[ 4 ] = { mRectColor.x, mRectColor.y, mRectColor.z, mRectColor.w };
		ImGui::DragFloat4( "Rect Color", col, 0.01f, 0.0f, 1.0f );
		mRectColor = Vec4( col[ 0 ], col[ 1 ], col[ 2 ], col[ 3 ] );

		if ( ImGui::TreeNode( "Application" ) )
		{
			if ( mProject.GetApplication() )
			{
				igm->DebugDumpObject( mProject.GetApplication() ); 
			}
			ImGui::TreePop( );
		} 

		if ( ImGui::TreeNode( "World Time" ) )
		{
			WorldTime wt = Engine::GetInstance( )->GetWorldTime( ); 
			if ( ImGui::SliderFloat( "Time Scale", &wt.mTimeScale, 0.001, 1.0f ) )
			{
				Engine::GetInstance( )->SetWorldTimeScale( wt.mTimeScale );
			}
			ImGui::TreePop( );
		}
	}

	void EditorApp::PlayOptions( )
	{
		if ( mPlaying )
		{
			if ( ImGui::Button( "Stop" ) )
			{ 
				mPlaying = false; 
				mMoveCamera = false;

				// Call shut down function for game
				if ( mProject.GetApplication() )
				{
					// Need to pass in previous scene to restore...
					ShutdownProjectApp( nullptr );
				}

				GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
				auto cam = gfx->GetGraphicsSceneCamera( )->ConstCast< Camera >();
				cam->SetPosition( mPreviousCameraTransform.GetPosition() );
				cam->SetRotation( mPreviousCameraTransform.GetRotation() ); 
			} 

			if ( mState != ApplicationState::Paused )
			{
				ImGui::SameLine( );
				if ( ImGui::Button( "Pause" ) )
				{
					SetApplicationState( ApplicationState::Paused );
					PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
					physx->PauseSystem( true );
				} 
			} 
			else
			{
				ImGui::SameLine( );
				if ( ImGui::Button( "Resume" ) )
				{
					SetApplicationState( ApplicationState::Running );
					PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
					physx->PauseSystem( false );
				} 
			}
		}
		else
		{
			// Do not play unless a scene is available!
			
			AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );
			if ( scene && !scene->IsDefault() )
			{ 
				if ( ImGui::Button( "Play" ) )
				{ 
					mPlaying = true;
					mMoveCamera = true;

					GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
					auto cam = gfx->GetGraphicsSceneCamera( );
					mPreviousCameraTransform = Enjon::Transform( cam->GetPosition(), cam->GetRotation(), Enjon::Vec3( cam->GetOrthographicScale() ) ); 

					// Call start up function for game
					if ( mProject.GetApplication() )
					{
						InitializeProjectApp( );
					}
					else
					{ 
						std::cout << "Cannot play without game loaded!\n";
						mPlaying = false;
						mMoveCamera = false;
					}
				} 
			}
			else
			{
				ImVec4 buttonCol = ImVec4( 0.25f, 0.25f, 0.25f, 1.f );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.8f, 0.8f, 0.8f, 0.6f ) );
				ImGui::PushStyleColor( ImGuiCol_Button, buttonCol );
				ImGui::PushStyleColor( ImGuiCol_ButtonHovered, buttonCol );
				ImGui::PushStyleColor( ImGuiCol_ButtonActive, buttonCol );
				ImGui::Button( "Play" );
				ImGui::PopStyleColor( 4 );
			}

			ImGui::SameLine( );
			if ( ImGui::Button( "Reload" ) )
			{ 
				mNeedReload = true;
			}

			// Compile the project if available
			if ( mProject.GetApplication( ) )
			{
				ImGui::SameLine( ); 
				if ( ImGui::Button( "Compile" ) )
				{
					mNeedRecompile = true;
				} 

				ImGui::SameLine( );
				if ( ImGui::Button( "Build" ) )
				{
					Result res = mProject.BuildProject( );
				}

				ImGui::SameLine( );
				if ( ImGui::Button( "Simulate" ) )
				{
					Result res = mProject.BuildProject( );
					if ( res == Result::SUCCESS )
					{
						mProject.Simluate( );
					} 
				}
			} 
		} 
	}

	void EditorApp::SelectEntity( const EntityHandle& handle )
	{
		mWorldOutlinerView->SelectEntity( handle );
	}

	void EditorApp::DeselectEntity( )
	{
		mWorldOutlinerView->DeselectEntity( );
	}

	void EditorApp::LoadResourceFromFile( )
	{
		// Load file
		if ( ImGui::CollapsingHeader( "Load Resource" ) )
		{
			char buffer[ 256 ];
			std::strncpy( buffer, mResourceFilePathToLoad.c_str( ), 256 );
			if ( ImGui::InputText( "File Path", buffer, 256 ) )
			{
				mResourceFilePathToLoad = Enjon::String( buffer );
			}

			if ( ImGui::Button( "Load File" ) )
			{
				Enjon::AssetManager* am = EngineSubsystem( AssetManager );
				am->AddToDatabase( mResourceFilePathToLoad );
			}
		}
	}

	//================================================================================================================================

	void EditorApp::SelectSceneView( )
	{
		SceneManager* sm = EngineSubsystem( SceneManager );
		AssetHandle< Scene > scene = sm->GetScene( ); 
		String defaultText = scene.Get( ) == nullptr ? "Available Scenes..." : scene->GetName( );
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( ImColor( ImGui::GetStyle( ).Colors[ ImGuiCol_FrameBg ] ) ) );
		if ( ImGui::BeginCombo( "##Available Scenes", defaultText.c_str() ) )
		{ 
			// If there's a valid application set
			if ( mProject.GetApplication( ) )
			{
				// Get component list
				AssetManager* am = EngineSubsystem( AssetManager );
				const HashMap<String, AssetRecordInfo>* scenes = am->GetAssets<Scene>( );

				for ( auto& record : *scenes )
				{ 
					// Add component to mEntity
					if ( ImGui::Selectable( record.second.GetAssetName( ).c_str( ) ) )
					{ 
						// Get uuid of asset
						UUID uuid = record.second.GetAssetUUID( );

						// Load the scene
						sm->LoadScene( uuid ); 

						// Deselect entity
						mWorldOutlinerView->DeselectEntity( );
					} 
				} 
			}
			else
			{
				ImGui::Text( "No project loaded." );
			}

			ImGui::EndCombo( ); 
		}
		ImGui::PopStyleColor( );

		if ( sm->GetScene() )
		{
			if ( ImGui::Button( "Save Scene" ) )
			{
				sm->GetScene()->Save( );
			}
		}
	}

	//================================================================================================================================
	 
	void EditorApp::CreateNewProject( const String& projectName )
	{ 
		// Just output the source files for now... This is already going to get ugly, so need to split this all up pretty quickly
		String projectDir = mProjectsDir + "/" + projectName + "/";
		if ( !fs::exists( projectDir ) )
		{
			fs::create_directory( projectDir );
			fs::create_directory( projectDir + "Source/" );
			fs::create_directory( projectDir + "Assets/" );
			fs::create_directory( projectDir + "Proc/" );
			fs::create_directory( projectDir + "Build/" );
			fs::create_directory( projectDir + "Build/Generator/" );
			fs::create_directory( projectDir + "Build/Generator/Intermediate/" );
			fs::create_directory( projectDir + "Build/Generator/Linked/" );
		}

		String includeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#HEADERFILEBEGIN", "#HEADERFILEEND", mProjectSourceTemplate, false ), "#PROJECTNAME", projectName );
		String sourceFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::ParseFromTo( "#SOURCEFILEBEGIN", "#SOURCEFILEEND", mProjectSourceTemplate, false ), "#PROJECTNAME", projectName ); 
		String cmakeFile = Enjon::Utils::FindReplaceAll( Enjon::Utils::FindReplaceAll( mProjectCMakeTemplate, "#PROJECTNAME", projectName ), "#ENJONDIRECTORY", Enjon::Utils::FindReplaceAll( Engine::GetInstance( )->GetConfig( ).GetRoot( ), "\\", "/" ) );
		String delBatFile = mProjectDelBatTemplate;
		String buildAndRunFIle = mProjectBuildAndRunTemplate;

		// Write to file
		Enjon::Utils::WriteToFile( includeFile, projectDir + "Source/" + projectName + ".h" );
		Enjon::Utils::WriteToFile( sourceFile, projectDir + "Source/" + projectName + ".cpp" ); 
		Enjon::Utils::WriteToFile( cmakeFile, projectDir + "CMakeLists.txt" ); 
		Enjon::Utils::WriteToFile( delBatFile, projectDir + "Proc/" + "DelPDB.bat" ); 
		Enjon::Utils::WriteToFile( buildAndRunFIle, projectDir + "Proc/" + "BuildAndRun.bat" ); 
		Enjon::Utils::WriteToFile( mProjectBuildBatTemplate, projectDir + "Proc/" + "Build.bat" ); 
		Enjon::Utils::WriteToFile( "", projectDir + "Build/Generator/Linked/" + projectName + "_Generated.cpp" ); 
		Enjon::Utils::WriteToFile( projectDir + "\n" + Engine::GetInstance()->GetConfig().GetRoot(), projectDir + projectName + ".eproj" );
		Enjon::Utils::WriteToFile( mCompileProjectBatTemplate, projectDir + "Proc/" + "CompileProject.bat" );


		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// Start the projection solution
		s32 code = system( String( "call " + projectDir + "Proc/" + "Build.bat" + " " + Enjon::Utils::FindReplaceAll( projectDir, "/", "\\" ) + " " + projectName ).c_str() ); 
		if ( code == 0 )
		{
			// Unload previous project
			UnloadDLL( ); 

			// Create new project
			Project proj; 
			proj.SetProjectPath( projectDir );
			proj.SetProjectName( projectName );
			proj.SetEditor( this );

			// Compile the project
			proj.CompileProject( ); 

			// Add project to list
			mProjectsOnDisk.push_back( proj ); 

			// Load the new project
			LoadProject( proj ); 

			// Load the solution for the project
			LoadProjectSolution( );
		}
		else
		{
			std::cout << "Could not build project.\n";
		}
#endif

	}

	//================================================================================================================================

	void EditorApp::LoadProject( const Project& project )
	{ 
		SceneManager* sm = EngineSubsystem( SceneManager );

		sm->UnloadScene( );

		// Unload previous dll
		UnloadDLL( );

		// Set project
		mProject = project;

		// Load project dll
		LoadDLL( );

		// Reinitialize asset manager
		AssetManager* am = EngineSubsystem( AssetManager );
		am->Reinitialize( mProject.GetProjectPath( ) + "Assets/" ); 
	}

	//================================================================================================================================

	void EditorApp::LoadProjectSolution( )
	{
		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Error check the fuck out of this call
		// Is it possible to know whether or not this succeeded?
		s32 code = system( String( "start " + mProject.GetProjectPath() + "Build/" + mProject.GetProjectName() + ".sln" ).c_str() ); 
#endif
	}

	//================================================================================================================================

	void EditorApp::PreCreateNewProject( const String& projectName )
	{
		mPrecreateNewProject = true;
		mNewProjectName = projectName;
	}

	//================================================================================================================================

	void EditorApp::SelectProjectDirectoryView( )
	{
		char buffer[ 1024 ];
		strncpy( buffer, mProjectsDir.c_str( ), 1024 ); 
		if ( ImGui::Button( "Select Project Directory" ) )
		{
			nfdchar_t* outPath = NULL;
			nfdresult_t res = NFD_PickFolder( NULL, &outPath );
			if ( res == NFD_OKAY )
			{
				// Set the path now
				if ( fs::exists( outPath ) && fs::is_directory( outPath ) )
				{
					mProjectsDir = outPath;
					CollectAllProjectsOnDisk( ); 
					WriteEditorConfigFileToDisk( );
				} 
			}
		}
		ImGui::SameLine( );
		ImGui::Text( "Project Directory: %s", buffer ); 
	}

	//================================================================================================================================

	bool EditorApp::CreateProjectView( )
	{ 
		auto cleanUpAndLeave = [ & ] ( bool retVal )
		{
			ImGui::PopStyleColor( );
			return retVal;
		};

		// Push text color for entire screen
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.8f, 0.8f, 0.8f, 1.f ) );

		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		//ImGui::SetCursorPosX( ( ImGui::GetWindowWidth( ) - sz.x ) / 2.f );
		ImGui::Text( "Welcome to the" ); 
		ImGui::SameLine( );
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, 1.f ) );
		ImGui::Text( "Project Creation" );
		ImGui::PopStyleColor( );
		ImGui::SameLine( );
		ImGui::Text( "screen." );
		ImGui::NewLine( );

		char tmpBuffer[ 1024 ];
		strncpy( tmpBuffer, mProjectsDir.c_str(), 1024 ); 

		ImGui::Text( "Select a location for your project." );
		ImGui::NewLine( );

		ImGui::PushItemWidth( 300.f );
		if ( ImGui::InputText( "##ProjectDir", tmpBuffer, 256 ) )
		{
			if ( fs::exists( tmpBuffer ) )
			{
				mProjectsDir = String( tmpBuffer );
			}
		}
		ImGui::PopItemWidth( );
		ImGui::SameLine( );
		ImGui::SetCursorPosX( ImGui::GetCursorPosX( ) - 10.f ); 
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.3f, 0.3f, 0.3f, 1.f ) );
		if ( ImGui::Button( "..." ) )
		{
			// Open file picking dialogue
			nfdchar_t* folder;
			nfdresult_t res = NFD_PickFolder( NULL, &folder );
			if ( res == NFD_OKAY )
			{
				mProjectsDir = String( folder );
			}
		}
		ImGui::PopStyleColor( );

		ImGui::SameLine( );
		strncpy( tmpBuffer, mNewProjectName.c_str( ), 1024 ); 
		u8 buffer[ 2048 ];
		snprintf( (char*)buffer, 2048, "%s/%s", mProjectsDir.c_str( ), tmpBuffer );
		b32 projExists = fs::exists( buffer ); 
		ImGui::PushItemWidth( 150.f );
		if ( ImGui::InputText( "Project Name", tmpBuffer, 50 ) )
		{ 
			snprintf( (char*)buffer, 2048, "%s/%s", mProjectsDir.c_str( ), tmpBuffer );
			if ( fs::exists( buffer ) )
			{
				projExists = true;
			}
			mNewProjectName = String( tmpBuffer ); 
		}
		ImGui::PopItemWidth( );

		if ( !projExists )
		{
			ImGui::NewLine( );
			if ( ImGui::Button( "Create New Project" ) && mNewProjectName.compare( "" ) != 0 )
			{
				// If project is able to be made, then make it
				String projectPath = mProjectsDir + "/" + mNewProjectName + "/";
				if ( !fs::exists( projectPath ) )
				{
					//CreateNewProject( mNewProjectName );
					PreCreateNewProject( mNewProjectName );
					return cleanUpAndLeave( true );
				}
				else
				{
					std::cout << "Project already exists!\n";
				} 
			} 
		}
		// Project already exist, so display warning
		else
		{
			ImGui::NewLine( );
			ImGui::SetCursorPosY( ImGui::GetCursorPosY( ) + 10.f );
			ImDrawList* dl = ImGui::GetWindowDrawList( );
			ImVec2 a = ImGui::GetCursorScreenPos( );
			ImVec2 b = ImVec2( a.x + 400.f, a.y + 40.f );
			dl->AddRectFilled( a, b, ImColor( 0.9f, 0.1f, 0.f, 1.0f ) );
			ImGui::Text( "Project already exists. Please choose a different project name." );
		} 

		return cleanUpAndLeave( false );
	}

	//================================================================================================================================

	void EditorApp::CleanupScene( )
	{ 
		// Force the scene to clean up ahead of frame
		EntityManager* entities = EngineSubsystem( EntityManager );
		entities->ForceCleanup( );
	}

	//================================================================================================================================

	bool EditorApp::UnloadDLL( ByteBuffer* buffer )
	{
		bool needsReload = false;

		if ( dllHandle )
		{
			if ( mPlaying && buffer )
			{
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					Enjon::ObjectArchiver::Serialize( app, buffer );
					needsReload = true;

					// Shutdown project app
					ShutdownProjectApp( buffer );
				}
			}

			// Free application memory
			if ( deleteAppFunc )
			{
				deleteAppFunc( mProject.GetApplication( ) );
				mProject.SetApplication( nullptr );
			}

			// Free library if in use
			FreeLibrary( dllHandle );
			dllHandle = nullptr;
			createAppFunc = nullptr;
			deleteAppFunc = nullptr;
		}

		return needsReload;
	}

	//================================================================================================================================

	void EditorApp::ReloadDLL( )
	{
		// Save the current scene and store uuid
		UUID sceneUUID;
		UUID selectedEntityUUID;
		SceneManager* sm = EngineSubsystem( SceneManager );
		EntityManager* em = EngineSubsystem( EntityManager );

		// Unload all archetypes
		EngineSubsystem( AssetManager )->UnloadAssets< Archetype >( );

		if ( mWorldOutlinerView->GetSelectedEntity() )
		{
			selectedEntityUUID = mWorldOutlinerView->GetSelectedEntity().Get( )->GetUUID();
		}

		if ( sm->GetScene() )
		{
			sceneUUID = sm->GetScene()->GetUUID( );
			sm->UnloadScene( );
		} 

		// Destroy all archetype roots 
		for ( auto& e : em->GetEntitiesByWorld( em->GetArchetypeWorld( ) ) )
		{
			e->ForceDestroy( );
		}

		// Force all windows to destroy that need it
		EngineSubsystem( WindowSubsystem )->ForceCleanupWindows( );

		// ReloadDLL without release scene asset
		LoadDLL( false ); 

		// Set current scene using previous id if valid
		if ( sceneUUID )
		{
			sm->LoadScene( sceneUUID );
		} 

		// Reselect the previous entity
		if ( selectedEntityUUID )
		{
			mWorldOutlinerView->SelectEntity( em->GetEntityByUUID( selectedEntityUUID ) );
		} 

		mNeedReload = false;
	}

	//================================================================================================================================

	void EditorApp::LoadDLL( bool releaseSceneAsset )
	{
		Enjon::ByteBuffer buffer;

		// Unload previous scene
		EngineSubsystem( SceneManager )->UnloadScene( );
		// Unload previous scene 
		//UnloadScene( releaseSceneAsset ); 

		// Actual code starts here...
		bool needsReload = UnloadDLL( &buffer );

		// Copy files to directory
		CopyLibraryContents( mProject.GetProjectName(), mProject.GetProjectPath() );

		// Try to load library
		dllHandle = LoadLibrary( ( mProject.GetProjectName() + ".dll" ).c_str() );
		//dllHandle = LoadLibrary( ( mProject.GetProjectPath() + "Build/" + configuration +"/" + mProject.GetProjectName() + ".dll" ).c_str() );

		// If valid, then set address of procedures to be called
		if ( dllHandle )
		{
			// Set functions from handle
			createAppFunc = ( funcCreateApp )GetProcAddress( dllHandle, "CreateApplication" );
			deleteAppFunc = ( funcDeleteApp )GetProcAddress( dllHandle, "DeleteApplication" );

			// Create application
			if ( createAppFunc )
			{
				mProject.SetApplication( createAppFunc( Enjon::Engine::GetInstance( ) ) );
			} 

			// Reload the entity from the buffer
			if ( buffer.GetSize( ) )
			{
				// Deserialize scene entity
				mSceneEntity = Enjon::EntityArchiver::Deserialize( &buffer ); 

				// Push back scene entity into mSceneEntities
				mSceneEntities.push_back( mSceneEntity.Get( ) );
			} 

			if ( needsReload )
			{
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					std::cout << "Buffer size: " << buffer.GetSize( ) << "\n";
					Enjon::ObjectArchiver::Deserialize( &buffer, app ); 

					for ( u32 i = 0; i < 30; ++i )
					{
						// Write this out to file for shiggles
						buffer.WriteToFile( mProject.GetProjectPath( ) + "/testWriteBuffer" + fmt::format("{}", i ) ); 
					}
				}
			}
		}
		else
		{
			// Reload the entity from the buffer
			if ( buffer.GetSize( ) )
			{
				// Deserialize scene entity
				mSceneEntity = Enjon::EntityArchiver::Deserialize( &buffer );

				// Push back scene entity into mSceneEntities
				mSceneEntities.push_back( mSceneEntity.Get( ) );
			}

			std::cout << "Could not load library\n";
		}

		// Reload current scene
		SceneManager* sm = EngineSubsystem( SceneManager );
		sm->ReloadScene( );
	}

	//================================================================================================================================

	String EditorApp::GetBuildConfig( ) const
	{
		return configuration;
	}

	//================================================================================================================================

	String EditorApp::GetVisualStudioDirectoryPath( ) const
	{
		return mVisualStudioDir;
	}

	//================================================================================================================================

	String EditorApp::GetCompileProjectCMakeTemplate( ) const
	{
		return mCompileProjectCMakeTemplate;
	}

	//================================================================================================================================

	String EditorApp::GetBuildAndRunCompileTemplate( ) const
	{
		return mProjectBuildAndRunCompileTemplate;
	}

	//================================================================================================================================

	String EditorApp::GetProjectEnjonDefinesTemplate( ) const
	{
		return mProjectEnjonDefinesTemplate;
	}

	//================================================================================================================================

	String EditorApp::GetProjectMainTemplate( ) const
	{
		return mProjectMainTemplate;
	} 

	//================================================================================================================================

	void EditorApp::WriteEditorConfigFileToDisk( )
	{
		// Write out the config file for the editor (Just do this as json later on for ease of use) 
		// I want these things to be const char* instead. I'm wasting memory with all of this junk.
		Utils::WriteToFile( mProjectsDir, fs::current_path().string() + "/editor.cfg");
	}

	//================================================================================================================================ 

	void EditorApp::CollectAllProjectsOnDisk( )
	{ 
		// So before we get to this point, need to make sure we HAVE a projects directory
		// In the directory for the editor, need to have a .ini or .config file that tells where the project directory is located 
		mProjectsOnDisk.clear( );

		for ( auto& p : fs::recursive_directory_iterator( mProjectsDir ) )
		{
			if ( Enjon::Utils::HasFileExtension( p.path( ).string( ), "eproj" ) )
			{ 
				Project proj;
				String path = "";
				Vector<String> split = Utils::SplitString( Utils::FindReplaceAll( Utils::SplitString( p.path( ).string( ), "." ).front( ), "\\", "/" ), "/" );
				split.pop_back( );
				for ( auto& p : split )
				{
					path += p + "/";
				}

				proj.SetProjectPath( path );
				proj.SetProjectName( Enjon::Utils::SplitString( Enjon::Utils::SplitString( p.path( ).string( ), "\\" ).back(), "." ).front() );
				proj.SetEditor( this );
				mProjectsOnDisk.push_back( proj );
			}
		} 
	}

	//================================================================================================================================

	void EditorApp::FindProjectOnLoad( )
	{
		if ( mProjectOnLoad.empty( ) )
		{
			return;
		}

		for ( auto& p : mProjectsOnDisk )
		{
			if ( p.GetProjectName( ).compare( mProjectOnLoad ) == 0 )
			{
				LoadProject( p );
				return;
			}
		}
	}

	void EditorApp::SetProjectOnLoad( const String& projectDir )
	{
		mProjectOnLoad = projectDir;
	}

	void EditorApp::InitializeProjectApp( )
	{
		// Get project application
		Application* app = mProject.GetApplication( );

		// Get current loaded scene
		AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );

		// If both are loaded then can initialize application
		if ( app && scene )
		{
			// Set application state to running
			SetApplicationState( ApplicationState::Running );

			// Cache off all entity handles in scene before app starts
			EntityManager* em = EngineSubsystem( EntityManager );
			mSceneEntities = em->GetActiveEntities( );

			// NOTE(): Don't really like this at all...
			// Save current scene before starting so we can reload on stop 
			scene.Save( );

			// Initialize the app
			app->Initialize( );

			// Turn on the physics simulation
			PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
			physx->PauseSystem( false ); 

			// Turn off the selection widget
			mTransformWidget.Enable( false );
		}
	}

	//================================================================================================================================

	void EditorApp::ShutdownProjectApp( ByteBuffer* buffer )
	{
		Application* app = mProject.GetApplication( );
		if ( app )
		{
			// Shutdown application state
			SetApplicationState( ApplicationState::Stopped ); 

			EntityManager* em = EngineSubsystem( EntityManager );
			SceneManager* sm = EngineSubsystem( SceneManager );
			Vector<Entity*> entities = em->GetActiveEntities( ); 

			// Shutodwn the application
			app->Shutdown( ); 

			// Unload current scene and catch its uuid
			UUID uuid = sm->UnloadScene( );

			// Reload scene with previous uuid
			sm->LoadScene( uuid );

			// Clean up physics subsystem from contact events as well
			PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
			phys->Reset( );

			// Pause the physics simulation
			phys->PauseSystem( true ); 
		}

		// Set the active camera in graphics scene to editor camera
		if ( mEditorCamera.GetGraphicsScene( ) )
		{
			mEditorCamera.GetGraphicsScene( )->SetActiveCamera( &mEditorCamera );
		}
	}

	//================================================================================================================

	void EditorApp::RegisterReloadDLLCallback( const ReloadDLLCallback& callback )
	{
		mReloadDLLCallbacks.push_back( callback );
	}

	//================================================================================================================

	void EditorApp::CleanupGUIContext( )
	{
		if ( mEditorSceneView ) { delete ( mEditorSceneView ); mEditorSceneView = nullptr; } 
		if ( mWorldOutlinerView ) { delete( mWorldOutlinerView ); mWorldOutlinerView = nullptr; } 
		if ( mAssetBroswerView ) { delete( mAssetBroswerView ); mAssetBroswerView = nullptr; }
		if ( mInspectorView ) { delete( mInspectorView ); mInspectorView = nullptr; }
		if ( mTransformToolBar ) { delete( mTransformToolBar ); mTransformToolBar = nullptr; }

		Window* mainWindow = EngineSubsystem( GraphicsSubsystem )->GetMainWindow( );
		assert( mainWindow != nullptr );

		GUIContext* guiContext = mainWindow->GetGUIContext( );
		assert( guiContext->GetContext( ) != nullptr );

		// Clear previous context
		guiContext->ClearContext( ); 
	}

	//================================================================================================================

	void EditorApp::LoadProjectContext( )
	{
		mPreloadProjectContext = false;

		Window* mainWindow = EngineSubsystem( GraphicsSubsystem )->GetMainWindow( );
		assert( mainWindow != nullptr );

		GUIContext* guiContext = mainWindow->GetGUIContext( );
		assert( guiContext->GetContext( ) != nullptr );

		// Set window title to whatever the project is
		mainWindow->SetWindowTitle( "Enjon Editor: " + mProject.GetProjectName( ) );

		// Destroy previous contexts and windows if available
		CleanupGUIContext( ); 

		// Add main menu options ( order matters )
		guiContext->RegisterMainMenu( "File" );
		guiContext->RegisterMainMenu( "Edit" );
		guiContext->RegisterMainMenu( "Create" );
		guiContext->RegisterMainMenu( "View" );

		// Add all necessary views into editor widget manager
		mEditorSceneView = new EditorViewport( this, mainWindow, "Viewport" );
		mWorldOutlinerView = new EditorWorldOutlinerView( this, mainWindow );
		mAssetBroswerView = new EditorAssetBrowserView( this, mainWindow );
		mInspectorView = new EditorInspectorView( this, mainWindow );
		mTransformToolBar = new EditorTransformWidgetToolBar( this, mainWindow );

		// Archetype callback for scene view
		mEditorSceneView->SetViewportCallback( ViewportCallbackType::AssetDropArchetype, [ & ] ( const void* data )
		{ 
			Archetype* archType = ((const Asset*)data)->ConstCast< Archetype >( );
			if ( archType )
			{
				// Instantiate the archetype right in front of the camera for now
				GraphicsSubsystemContext* gfxCtx = mEditorSceneView->GetWindow( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( );
				Camera* cam = gfxCtx->GetGraphicsScene( )->GetActiveCamera( );
				Vec3 position = cam->GetPosition() + cam->Forward( ) * 5.0f; 
				Vec3 scale = archType->GetRootEntity( ).Get( )->GetLocalScale( );
				EntityHandle handle = archType->Instantiate( Transform( position, Quaternion( ), scale ), mEditorSceneView->GetWindow()->GetWorld() );
			}
		});

		mEditorSceneView->SetViewportCallback( ViewportCallbackType::CustomRenderOverlay, [ & ] ( const void* data )
		{
			// Data should be null here
			// Just want to place the scene name at the bottom of the screen
			ImVec2 windowPos = ImGui::GetWindowPos( );
			ImVec2 windowSize = ImGui::GetWindowSize( ); 
			const Vec2 margin = Vec2(15.f, 12.f);
			String txt;
			ImVec2 txtSz;

			AssetHandle< Scene > currentScene = EngineSubsystem( SceneManager )->GetScene( ); 
			if ( currentScene )
			{
				txt = fmt::format( "Scene: {}", currentScene->GetName() );
				txtSz = ImGui::CalcTextSize( txt.c_str() );
			} 
			else
			{ 
				txt = fmt::format( "Scene: default" );
				txtSz = ImGui::CalcTextSize( txt.c_str() );
			}

			ImGui::SetCursorScreenPos( ImVec2( windowPos.x + windowSize.x - txtSz.x - margin.x, windowPos.y + windowSize.y - txtSz.y - margin.y ) );
			ImGui::Text( "%s", txt.c_str() );
		});

		// Register selection callback with outliner view
		mWorldOutlinerView->RegisterEntitySelectionCallback( [ & ] ( const EntityHandle& handle )
		{
			// If handle is valid, then we'll enable transforms and widgets
			if ( handle.Get() )
			{
				// Enable transform widget
				mTransformWidget.Enable( true );

				// Set transform to selected entity
				mTransformWidget.SetPosition( handle.Get( )->GetWorldPosition( ) ); 
				mTransformWidget.SetRotation( handle.Get( )->GetWorldRotation( ) ); 

				// Set selected object in inspector view
				mInspectorView->SetInspetedObject( handle.Get( ) );
			} 
		} );

		mWorldOutlinerView->RegisterEntityDeselectionCallback( [ & ] ( )
		{
			// Uninspect object
			mInspectorView->DeselectInspectedObject( ); 

			// Deactivate transform widget
			mTransformWidget.Enable( false ); 
		} );

		// Initialize transform widget
		mTransformWidget.Initialize( mEditorSceneView ); 

		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		guiContext->RegisterWindow( "Play Options", [ & ]
		{
			if ( ImGui::BeginDock( "Play Options", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize ) )
			{
				PlayOptions( );
				//CheckForPopups( );
			}
			ImGui::EndDock( );
		} );

		auto createEntity = [ & ] ( EditorApp* app )
		{
			if ( ImGui::MenuItem( "    Entity##options", NULL ) )
			{
				std::cout << "Creating entity...\n";
				EntityManager* em = EngineSubsystem( EntityManager );
				GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
				EntityHandle entity = em->Allocate( );
				if ( entity )
				{
					Entity* ent = entity.Get( ); 
					const Camera* cam = gs->GetGraphicsSceneCamera( );
					ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f ); 
					ent->SetName( "Entity" );
				}

				// Select entity
				app->mWorldOutlinerView->SelectEntity( entity );
			} 
		};

		auto createCamera = [ & ] ( EditorApp* app )
		{
			if ( ImGui::MenuItem( "    Camera##options", NULL ) )
			{
				std::cout << "Creating camera...\n";
				EntityManager* em = EngineSubsystem( EntityManager );
				GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
				EntityHandle entity = em->Allocate( );
				if ( entity )
				{ 
					Entity* ent = entity.Get( ); 
					CameraComponent* cc = ent->AddComponent< CameraComponent >( );

					const Camera* cam = gs->GetGraphicsSceneCamera( );
					ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f ); 
					ent->SetName( "Camera" );
				}

				// Select entity
				app->mWorldOutlinerView->SelectEntity( entity );
			} 
		};

		auto createLights = [ & ] ( EditorApp* app )
		{
			if ( ImGui::MenuItem( "    Point Light##options", NULL ) )
			{
				std::cout << "Creating point light...\n";
				EntityManager* em = EngineSubsystem( EntityManager );
				GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
				EntityHandle pointLight = em->Allocate( );
				if ( pointLight )
				{
					Entity* ent = pointLight.Get( ); 
					PointLightComponent* plc = ent->AddComponent<PointLightComponent>( );

					const Camera* cam = gs->GetGraphicsSceneCamera( );
					ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f ); 
					ent->SetName( "PointLight" );
				}

				// Select entity
				app->mWorldOutlinerView->SelectEntity( pointLight );
			}

			if ( ImGui::MenuItem( "    Directional Light##options", NULL ) )
			{
				std::cout << "Creating directional light...\n";
				EntityManager* em = EngineSubsystem( EntityManager );
				GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
				EntityHandle directionalLight = em->Allocate( );
				if ( directionalLight )
				{
					Entity* ent = directionalLight.Get( ); 
					ent->AddComponent<DirectionalLightComponent>( );

					const Camera* cam = gs->GetGraphicsSceneCamera( );
					ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );

					ent->SetName( "DirectionalLight" );
				}

				// Select entity
				app->mWorldOutlinerView->SelectEntity( directionalLight );
			} 
		};

		auto createPrimitives = [ & ] ( EditorApp* app )
		{ 
			if ( ImGui::BeginMenu( "    Primitives##options" ) )
			{
				if ( ImGui::MenuItem( "   Empty##options", NULL ) )
				{
					std::cout << "Creating empty entity...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					EntityHandle empty = em->Allocate( );
					empty.Get( )->SetName( "Empty" );

					// Set to selected entity
					app->mWorldOutlinerView->SelectEntity( empty );
				}

				if ( ImGui::MenuItem( "   Cube##options", NULL ) )
				{
					std::cout << "Creating cube...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle cube = em->Allocate( );
					if ( cube )
					{
						Entity* ent = cube.Get( );
						StaticMeshComponent* gfx = ent->AddComponent<StaticMeshComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ), 0 );

						RigidBodyComponent* rbc = ent->AddComponent<RigidBodyComponent>( );
						rbc->SetShape( CollisionShapeType::Box ); 

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );

						ent->SetName( "Cube" );
					}

					// Select entity
					app->mWorldOutlinerView->SelectEntity( cube );
				}

				if ( ImGui::MenuItem( "   Sphere##options", NULL ) )
				{
					std::cout << "Creating sphere...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle sphere = em->Allocate( );
					if ( sphere )
					{
						Entity* ent = sphere.Get( );
						StaticMeshComponent* gfx = ent->AddComponent<StaticMeshComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_sphere" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ), 0 );

						RigidBodyComponent* rbc = ent->AddComponent< RigidBodyComponent >( );
						rbc->SetShape( CollisionShapeType::Sphere );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );

						ent->SetName( "Sphere" );
					}

					// Select entity
					app->mWorldOutlinerView->SelectEntity( sphere );
				}

				ImGui::EndMenu( );
			}
		};

		auto createViewOption = [&]()
		{
			if ( mProject.GetApplication() && !mPlaying )
			{
				ImGuiManager* igm = EngineSubsystem( ImGuiManager );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 0.3f, 0.3f, 1.f ) );
				igm->Text( "GENERAL" );
				ImGui::PopStyleColor( );
				createEntity( this );
				createCamera( this );
				createPrimitives( this );

				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 0.3f, 0.3f, 1.f ) );
				igm->Text( "LIGHTING" );
				ImGui::PopStyleColor( );
				createLights( this );
			}
		}; 

		auto saveSceneOption = [ & ] ( )
		{ 
			AssetHandle< Scene > currentScene = EngineSubsystem( SceneManager )->GetScene( ); 
			ImColor textColor = ImGui::GetColorU32( ImGuiCol_Text ); 
			bool sceneValid = currentScene.IsValid( );
			if ( !sceneValid )
			{
				textColor.Value = ImVec4( 0.2f, 0.2f, 0.2f, 1.0f ); 
			}
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(textColor) );
			if ( ImGui::MenuItem( "Save Scene##options", NULL ) )
			{
				if ( sceneValid )
				{
					currentScene->Save( );
				} 
			}
			ImGui::PopStyleColor( );

			if ( ImGui::MenuItem( "Load Project##options", NULL ) )
			{
				SceneManager* sm = EngineSubsystem( SceneManager );

				// Must save current scene
				AssetHandle< Scene > currentScene = sm->GetScene( );
				if ( currentScene )
				{
					currentScene->Save( );
				}

				// Unload current scene
				sm->UnloadScene( );

				mNeedsLoadProject = true;
			}
		};

		// Register menu options
		//guiContext->RegisterMenuOption("File", "Load Project...##options", loadProjectMenuOption); 
		guiContext->RegisterMenuOption("File", "Save Scene##options", saveSceneOption); 
		guiContext->RegisterMenuOption( "Create", "Create", createViewOption );

		static bool mShowStyles = false;
		auto stylesMenuOption = [&]()
		{
        	ImGui::MenuItem("Styles##options", NULL, &mShowStyles);
		};
	 	auto showStylesWindowFunc = [&]()
	 	{
			if (ImGui::BeginDock("Styles##options", &mShowStyles))
			{
				ImGui::ShowStyleEditor();	
			}
			ImGui::EndDock();
	 	}; 

		guiContext->RegisterMenuOption("View", "Styles##Options", stylesMenuOption);
		guiContext->RegisterWindow("Styles", showStylesWindowFunc); 

		// Register docking layouts 
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Viewport", nullptr, GUIDockSlotType::Slot_Top, 1.0f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Play Options", "Viewport", GUIDockSlotType::Slot_Top, 0.1f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "World Outliner", nullptr, GUIDockSlotType::Slot_Right, 0.3f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Transform ToolBar", "Play Options", GUIDockSlotType::Slot_Right, 0.7f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Inspector", "World Outliner", GUIDockSlotType::Slot_Bottom, 0.6f ) );
		guiContext->RegisterDockingLayout( GUIDockingLayout( "Asset Browser", "Viewport", GUIDockSlotType::Slot_Bottom, 0.3f ) ); 

		guiContext->Finalize( );
	}

	//================================================================================================================

	void EditorApp::ProjectListView( )
	{ 
		// I don't like this...
		b32 projDirSet = fs::v1::exists( mProjectsDir );

		if ( ImGui::BeginDock( "Project Browser", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize ) )
		{
			if ( projDirSet )
			{
				String defaultText = mProject.GetApplication( ) == nullptr ? "Existing Projects..." : mProject.GetProjectName( );
				if ( ImGui::BeginCombo( "##LOADPROJECTLIST", defaultText.c_str() ) )
				{
					for ( auto& p : mProjectsOnDisk )
					{
						if ( ImGui::Selectable( p.GetProjectName( ).c_str( ) ) )
						{ 
							// Preload project for next frame
							PreloadProject( p );
							WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
							ws->DestroyWindow( mProjectSelectionWindow );
						}
					}
					ImGui::EndCombo( );
				} 
			}
			else
			{
				// Load view for project
				ImGui::Text( "Welcome to the Enjon Editor! You do not have a project directory selected currently. Please choose one now." );
				if ( ImGui::Button( "Choose Project Directory" ) )
				{
					nfdchar_t* outPath = NULL;
					nfdresult_t res = NFD_PickFolder( NULL, &outPath );
					if ( res == NFD_OKAY )
					{
						// Set the path now
						if ( fs::exists( outPath ) && fs::is_directory( outPath ) )
						{
							mProjectsDir = outPath;
							CollectAllProjectsOnDisk( ); 
							WriteEditorConfigFileToDisk( );
						} 
					}
				}
			}
		}
		ImGui::EndDock( ); 
	}

	//================================================================================================================

	void EditorApp::NewProjectView( )
	{ 
		if ( ImGui::BeginDock( "New Project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize ) )
		{
			if ( CreateProjectView( ) )
			{
				WindowSubsystem* ws = EngineSubsystem( WindowSubsystem );
				ws->DestroyWindow( mProjectSelectionWindow );
			}
		}
		ImGui::EndDock( );
	}

	//================================================================================================================

	void EditorApp::LoadProjectSelectionContext( )
	{
		// Set the window to small
		Window* window = EngineSubsystem( GraphicsSubsystem )->GetMainWindow( );
		assert( window != nullptr );

		// Set the size of the window
		window->HideWindow( );

		WindowParams params;
		params.mWindowClass = Object::GetClass< Window >();
		params.mWidth = 1200;
		params.mHeight = 500;
		params.mName = "Enjon: Project Browser";;
		EngineSubsystem( WindowSubsystem )->AddNewWindow( params );
		EngineSubsystem( WindowSubsystem )->ForceInitWindows( );
		mProjectSelectionWindow = EngineSubsystem( WindowSubsystem )->GetWindows( ).back( );

		GUIContext* guiCtx = mProjectSelectionWindow->GetGUIContext( );

		String curDir = fs::current_path( ).string( );
		if ( !fs::exists( mProjectsDir ) && fs::exists( curDir + "/editor.cfg" ) )
		{ 
			String config = Utils::read_file_sstream( "editor.cfg" );
			mProjectsDir = config;
			CollectAllProjectsOnDisk( );
		} 

		// View all projects loaded currently. This list should be held in a project manifest for the editor.
		auto createProjectView = [ & ] ( )
		{ 
			ProjectListView( );
		}; 

		auto newProjectView = [ & ] ( )
		{
			NewProjectView( );
		};

		guiCtx->RegisterWindow( "Projects", createProjectView ); 
		guiCtx->RegisterWindow( "New Project", newProjectView ); 
		guiCtx->RegisterDockingLayout( GUIDockingLayout( "Project Browser", nullptr, GUIDockSlotType::Slot_Top, 1.0f ) ); 
		guiCtx->RegisterDockingLayout( GUIDockingLayout( "New Project", nullptr, GUIDockSlotType::Slot_Tab, 1.0f ) ); 
		guiCtx->SetActiveDock( "Project Browser" );

		guiCtx->Finalize( );
	}

	//================================================================================================================

	void EditorApp::PreloadProject( const Project& project )
	{
		mPreloadProjectContext = true;
		mProject = project;
	}

	//================================================================================================================
	 
	Enjon::Result EditorApp::Initialize( )
	{ 
		mApplicationName = "EditorApp"; 

		// Set application state to stopped by default
		SetApplicationState( ApplicationState::Stopped );

		Enjon::String mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + "Editor/Assets/";

		// Get asset manager and set its properties ( I don't like this )
		AssetManager* mAssetManager = EngineSubsystem( AssetManager );
		GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem ); 

		// Set up camera and then add to graphics scene
		mEditorCamera = Camera( mGfx->GetViewport() );
		mEditorCamera.SetNearFar( 0.1f, 1000.0f );
		mEditorCamera.SetProjection(ProjectionType::Perspective);
		mEditorCamera.SetPosition(Vec3(0, 5, 10)); 
		mGfx->GetGraphicsScene( )->AddCamera( &mEditorCamera );
		mGfx->GetGraphicsScene()->SetActiveCamera( &mEditorCamera );

		// Pause the physics simulation
		physx->PauseSystem( true ); 

		// Register project template files
		mProjectSourceTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectSourceTemplate.cpp" ).c_str() ); 
		mProjectCMakeTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectCMakeTemplate.txt" ).c_str( ) );
		mProjectDelBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/DelPDB.bat" ).c_str( ) );
		mProjectBuildAndRunTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/BuildAndRun.bat" ).c_str( ) ); 
		mProjectBuildBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/Build.bat" ).c_str( ) ); 
		mComponentSourceTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ComponentSourceTemplate.cpp" ).c_str( ) ); 
		mCompileProjectBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/CompileProject.bat" ).c_str( ) ); 
		mCompileProjectCMakeTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectCompileCMakeTemplate.txt" ).c_str( ) ); 
		mProjectMainTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectAppMain.cpp" ).c_str( ) ); 
		mProjectBuildAndRunCompileTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/BuildAndRunCompile.bat" ).c_str( ) ); 
		mProjectEnjonDefinesTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectEnjonDefines.h" ).c_str( ) ); 

		// Set up copy directory for project dll
		copyDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + projectName + "/"; 

#if LOAD_ENGINE_RESOURCES
		LoadResources( );
#endif 
		// Search for loaded project  
		// NOTE(): ( this is hideous, by the way )
		FindProjectOnLoad( );

		if ( mProject.IsLoaded( ) || mPreloadProjectContext )
		{
			LoadProjectContext( );
			ReloadDLL( );
		}
		else
		{
			LoadProjectSelectionContext();
		}

		return Enjon::Result::SUCCESS;
	} 

	//=================================================================================================

	void EditorApp::CheckForPopups( )
	{
		if ( mLoadProjectPopupDialogue )
		{
			LoadProjectView( );
		}
	}
	
	Project* EditorApp::GetProject( )
	{
		return &mProject;
	}

	Enjon::Result EditorApp::Update( f32 dt )
	{ 
		if ( mPreloadProjectContext )
		{
			LoadProjectContext( );
			LoadProject( mProject );
			EngineSubsystem( GraphicsSubsystem )->GetMainWindow( )->ShowWindow( );
			mPreloadProjectContext = false;
		}

		if ( mPrecreateNewProject )
		{
			CreateNewProject( mNewProjectName );
			LoadProjectContext( );
			LoadProject( mProject );
			EngineSubsystem( GraphicsSubsystem )->GetMainWindow( )->ShowWindow( );
			mPrecreateNewProject = false;
		} 

		if ( mNeedReload )
		{
			// Reload the dll
			ReloadDLL( ); 
		}

		if ( mNeedRecompile )
		{
			Result res = mProject.CompileProject( );

			AssetHandle< Scene > scene = EngineSubsystem( SceneManager )->GetScene( );

			// Save the scene
			if ( scene )
			{
				scene->Save( );
			} 

			// Force reload the project after successful compilation
			if ( res == Result::SUCCESS )
			{
				ReloadDLL( );
			}

			mNeedRecompile = false;
		}

		if ( mNeedsLoadProject )
		{
			mNeedsLoadProject = false;
			LoadProjectSelectionContext( );
		}

		if ( !mProject.IsLoaded( ) )
		{
			return Result::PROCESS_RUNNING;
		}
		
		// Update transform widget
		mTransformWidget.Update( );

		// Simulate game tick scenario if playing
		if ( mPlaying )
		{
			Application* app = mProject.GetApplication( );
			if ( app )
			{
				// Process application input
				app->ProcessInput( dt );

				// Update application ( ^ Could be called in the same tick )
				app->Update( dt ); 
			}
		} 

		return Enjon::Result::PROCESS_RUNNING;
	}

	void EditorApp::EnableTransformSnapping( bool enable, const TransformationMode& mode )
	{
		mTransformWidget.EnableSnapping( enable, mode );
	}

	bool EditorApp::IsTransformSnappingEnabled( const TransformationMode& mode )
	{
		return mTransformWidget.IsSnapEnabled( mode );
	}

	f32 EditorApp::GetTransformSnap( const TransformationMode& mode )
	{
		switch ( mode )
		{
			case TransformationMode::Translation: { return mTransformWidget.GetTranslationSnap( ); } break;
			case TransformationMode::Rotation: { return mTransformWidget.GetRotationSnap( ); } break;
			case TransformationMode::Scale: { return mTransformWidget.GetScaleSnap( ); } break;
		}

		return 0.0f;
	}

	void EditorApp::SetTransformSnap( const TransformationMode& mode, const f32& val )
	{
		switch ( mode )
		{
			case TransformationMode::Translation: { mTransformWidget.SetTranslationSnap( val ); } break;
			case TransformationMode::Rotation: { mTransformWidget.SetRotationSnap( val ); } break;
			case TransformationMode::Scale: { mTransformWidget.SetScaleSnap( val ); } break;
		} 
	}

	Enjon::Result EditorApp::ProcessInput( f32 dt )
	{
		// NOTE( John ): HACK! Need to appropriately set up input states to keep this from occuring 
		if ( !mProject.IsLoaded( ) )
		{
			return Result::PROCESS_RUNNING;
		}

		GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		static Vec2 mMouseCoordsDelta = Vec2( 0.0f );
		Input* mInput = EngineSubsystem( Input );
		Camera* camera = mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( );
		Enjon::iVec2 viewPort = mGfx->GetViewport( ); 
		Enjon::Window* window = mGfx->GetWindow( )->ConstCast< Enjon::Window >( );

		// Can move camera if scene view has focus
		bool previousCamMove = mMoveCamera;
		mMoveCamera = mEditorSceneView->IsFocused( );
		if ( mMoveCamera && ( mMoveCamera != previousCamMove ) )
		{ 
			Vec2 mc = mInput->GetMouseCoords( );
			Vec2 center = mEditorSceneView->GetCenterOfViewport( );
			mMouseCoordsDelta = Vec2( (f32)(viewPort.x) / 2.0f - mc.x, (f32)(viewPort.y) / 2.0f - mc.y );
		}

		//if ( !mPlaying )
		{
			if ( !mMoveCamera )
			{
				if ( mInput->IsKeyPressed( KeyCode::Delete ) )
				{
					if ( mWorldOutlinerView->GetSelectedEntity() )
					{
						mWorldOutlinerView->GetSelectedEntity().Get( )->Destroy( );
						mWorldOutlinerView->DeselectEntity( );
					}
				}

				if ( mWorldOutlinerView->GetSelectedEntity() )
				{
					if ( mInput->IsKeyPressed( KeyCode::W ) )
					{
						mTransformWidget.SetTransformationMode( TransformationMode::Translation );
					}
					if ( mInput->IsKeyPressed( KeyCode::E ) )
					{
						mTransformWidget.SetTransformationMode( TransformationMode::Rotation );
					}
					if ( mInput->IsKeyPressed( KeyCode::R ) )
					{
						mTransformWidget.SetTransformationMode( TransformationMode::Scale );
					} 
					if ( mInput->IsKeyPressed( KeyCode::L ) )
					{
						switch ( mTransformWidget.GetTransformSpace( ) )
						{
							case TransformSpace::Local:
							{
								mTransformWidget.SetTransformSpace( TransformSpace::World ); 
							} break;
							case TransformSpace::World:
							{
								mTransformWidget.SetTransformSpace( TransformSpace::Local ); 
							} break;
						}
					}

					// Move entity to editor camera position and set rotation
					if ( mInput->IsKeyDown( KeyCode::LeftShift ) && mInput->IsKeyDown( KeyCode::LeftCtrl ) )
					{
						if ( mInput->IsKeyPressed( KeyCode::F ) )
						{
							mWorldOutlinerView->GetSelectedEntity().Get( )->SetWorldPosition( mEditorCamera.GetPosition( ) );
							mWorldOutlinerView->GetSelectedEntity().Get( )->SetWorldRotation( mEditorCamera.GetRotation( ).Normalize() ); 
						}
					}

					// Copy entity
					if ( mInput->IsKeyDown( KeyCode::LeftCtrl ) && mInput->IsKeyPressed( KeyCode::D ) )
					{
						EntityManager* em = EngineSubsystem( EntityManager );
						EntityHandle newEnt = em->CopyEntity( mWorldOutlinerView->GetSelectedEntity() );
						if ( newEnt )
						{
							mWorldOutlinerView->SelectEntity( newEnt );
						}
					}
				}

				// Interact with transform widget code
				if ( mInput->IsKeyDown( KeyCode::LeftMouseButton ) )
				{
					if ( mTransformWidget.IsInteractingWithWidget( ) )
					{
						Entity* ent = mWorldOutlinerView->GetSelectedEntity().Get( );
						Transform wt = ent->GetWorldTransform( ); 
						mTransformWidget.InteractWithWidget( &wt );
						ent->SetWorldTransform( wt ); 
						if ( ent->HasPrototypeEntity( ) )
						{
							ObjectArchiver::RecordAllPropertyOverrides( ent->GetPrototypeEntity( ).Get( ), ent );
						}
					} 
				}
				else
				{
					mTransformWidget.EndInteraction( );
				}
			}

			// Move scene view camera
			if ( mMoveCamera )
			{ 
				mEditorSceneView->UpdateCamera( );
			}

			// Mouse cursor on, interact with world
			else
			{
				//mGfx->GetMainWindow( )->ConstCast< Window >( )->ShowMouseCursor( true );
				if ( mEditorSceneView->IsHovered() )
				{
					if ( mInput->IsKeyPressed( KeyCode::LeftMouseButton ) )
					{
						auto viewport = mGfx->GetViewport( );
						auto mp = GetSceneViewProjectedCursorPosition( );

						iVec2 dispSize = mGfx->GetViewport( );
						PickResult pr = mGfx->GetPickedObjectResult( GetSceneViewProjectedCursorPosition( ), Engine::GetInstance( )->GetWorld( )->GetContext< GraphicsSubsystemContext >( ) );
						if ( pr.mEntity.Get( ) )
						{
							// Set selected entity
							mWorldOutlinerView->SelectEntity( pr.mEntity );
						}
						// Translation widget interaction
						else if ( EditorTransformWidget::IsValidID( pr.mId ) )
						{
							// Begin widget interaction
							mTransformWidget.BeginInteraction( TransformWidgetRenderableType( pr.mId - MAX_ENTITIES ), mWorldOutlinerView->GetSelectedEntity( ).Get( )->GetWorldTransform( ) );
						}
						else
						{
							// Deselect entity if click in scene view and not anything valid
							mWorldOutlinerView->DeselectEntity( ); 
						}
					} 
				}
			} 

			// NOTE(): Don't like having to do this here...
			if ( !mTransformWidget.IsInteractingWithWidget( ) && mWorldOutlinerView->GetSelectedEntity() )
			{
				Entity* ent = mWorldOutlinerView->GetSelectedEntity( ).Get( );
				mTransformWidget.SetPosition( ent->GetWorldPosition( ) ); 
				mTransformWidget.SetRotation( ent->GetWorldRotation( ) );
			}
		}

		// Starting /Stopping game instance
		if ( mInput->IsKeyPressed( Enjon::KeyCode::Escape ) )
		{
			if ( mPlaying )
			{
				mPlaying = false; 
				mMoveCamera = false;

				// Call shut down function for game
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					// Shutown the application
					ShutdownProjectApp( nullptr );
				}

				camera->SetPosition( mPreviousCameraTransform.GetPosition() );
				camera->SetRotation( mPreviousCameraTransform.GetRotation() );
			}
		} 

		return Enjon::Result::PROCESS_RUNNING;
	}

	Enjon::Result EditorApp::Shutdown( )
	{ 
		// Clean up all views
		if ( mEditorSceneView )
		{
			delete ( mEditorSceneView );
			mEditorSceneView = nullptr;
		}

		if ( mWorldOutlinerView )
		{
			delete ( mWorldOutlinerView );
			mWorldOutlinerView = nullptr;
		}

		if ( mAssetBroswerView )
		{
			delete ( mAssetBroswerView );
			mAssetBroswerView = nullptr;
		}

		if ( mInspectorView )
		{
			delete ( mInspectorView );
			mInspectorView = nullptr;
		}

		if ( mTransformToolBar )
		{
			delete( mTransformToolBar );
			mTransformToolBar = nullptr;
		}

		return Enjon::Result::SUCCESS;
	} 

	void EditorApp::LoadProjectResources( )
	{
		Enjon::String paintPeelingAlbedoPath = Enjon::String( "Materials/PaintPeeling/Albedo.png" );
		Enjon::String paintPeelingNormalPath = Enjon::String( "Materials/PaintPeeling/Normal.png" );
		Enjon::String paintPeelingRoughnessPath = Enjon::String( "Materials/PaintPeeling/Roughness.png" );
		Enjon::String paintPeelingMetallicPath = Enjon::String( "Materials/PaintPeeling/Metallic.png" );
		Enjon::String paintPeelingAOPath = Enjon::String( "Materials/PaintPeeling/ao.png" );

		AssetManager* am = EngineSubsystem( AssetManager );

		am->AddToDatabase( paintPeelingAlbedoPath );
		am->AddToDatabase( paintPeelingNormalPath );
		am->AddToDatabase( paintPeelingMetallicPath );
		am->AddToDatabase( paintPeelingRoughnessPath );
		am->AddToDatabase( paintPeelingAOPath );

		AssetHandle< Material > pp = am->ConstructAsset< Material >( );
		AssetHandle< ShaderGraph > sg = am->GetAsset< ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" ); 
		pp->SetShaderGraph( sg );
		pp.Get()->ConstCast< Material >()->SetUniform( "albedoMap", am->GetAsset< Texture >( "materials.paintpeeling.albedo" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "normalMap", am->GetAsset< Texture >( "materials.paintpeeling.normal" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "metallicMap", am->GetAsset< Texture >( "materials.paintpeeling.metallic" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "roughMap", am->GetAsset< Texture >( "materials.paintpeeling.roughness" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "aoMap", am->GetAsset< Texture >( "materials.paintpeeling.ao" ) );
		pp.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", am->GetAsset< Texture >( "textures.black" ) );
		pp->Save( );
	}

	void EditorApp::LoadResources( )
	{
		// Paths to resources
		Enjon::String greenPath				= Enjon::String("Textures/green.png"); 
		Enjon::String redPath				= Enjon::String("Textures/red.png"); 
		Enjon::String bluePath				= Enjon::String("Textures/blue.png"); 
		Enjon::String blackPath				= Enjon::String("Textures/black.png"); 
		Enjon::String midGreyPath			= Enjon::String("Textures/grey.png"); 
		Enjon::String lightGreyPath			= Enjon::String("Textures/light_grey.png"); 
		Enjon::String whitePath				= Enjon::String("Textures/white.png"); 
		Enjon::String yellowPath			= Enjon::String("Textures/yellow.png"); 
		Enjon::String axisBoxDiffusePath	= Enjon::String("Textures/axisBoxDiffuse.png"); 
		Enjon::String hdrPath				= Enjon::String("Textures/HDR/03-ueno-shrine_3k.hdr"); 
		Enjon::String hdrPath2				= Enjon::String("Textures/HDR/GCanyon_C_YumaPoint_3k.hdr"); 
		Enjon::String hdrPath3				= Enjon::String("Textures/HDR/WinterForest_Ref.hdr"); 
		Enjon::String cubePath				= Enjon::String("Models/unit_cube.obj"); 
		Enjon::String spherePath			= Enjon::String("Models/unit_sphere.obj"); 
		Enjon::String conePath				= Enjon::String("Models/unit_cone.obj"); 
		Enjon::String cylinderPath			= Enjon::String("Models/unit_cylinder.obj"); 
		Enjon::String ringPath				= Enjon::String("Models/unit_ring.obj"); 
		Enjon::String axisBoxPath			= Enjon::String("Models/axisBox.obj"); 
		Enjon::String shaderGraphPath		= Enjon::String("Shaders/ShaderGraphs/DefaultStaticGeom.sg"); 

		AssetManager* mAssetManager = EngineSubsystem( AssetManager );
		
		// Add to asset database( will serialize the asset if not loaded from disk, otherwise will load the asset )
		mAssetManager->AddToDatabase( axisBoxDiffusePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( greenPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( redPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( midGreyPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( lightGreyPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( bluePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( blackPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( whitePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( yellowPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( hdrPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( hdrPath2, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( hdrPath3, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( cubePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( spherePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( conePath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( cylinderPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( ringPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( axisBoxPath, true, true, AssetLocationType::EngineAsset );
		mAssetManager->AddToDatabase( shaderGraphPath, true, true, AssetLocationType::EngineAsset );

		// Create materials
		AssetHandle< Material > redMat = mAssetManager->ConstructAsset< Material >( "RedMaterial" );
		AssetHandle< Material > greenMat = mAssetManager->ConstructAsset< Material >( "GreenMaterial" );
		AssetHandle< Material > blueMat = mAssetManager->ConstructAsset< Material >( "BlueMaterial" );
		AssetHandle< Material > yellowMat = mAssetManager->ConstructAsset< Material >( "YellowMaterial" );
		AssetHandle< Material > axisBoxMat = mAssetManager->ConstructAsset< Material >( "AxisBoxMat" );
		AssetHandle< ShaderGraph > sg = mAssetManager->GetAsset< ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" );

		axisBoxMat->SetShaderGraph( sg );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.axisboxdiffuse" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		axisBoxMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		redMat->SetShaderGraph( sg );
		redMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.red" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		redMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		greenMat->SetShaderGraph( sg );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.green" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		greenMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		blueMat->SetShaderGraph( sg );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.blue" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		blueMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		yellowMat->SetShaderGraph( sg );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "albedoMap", mAssetManager->GetAsset< Texture >( "textures.yellow" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "normalMap", mAssetManager->GetAsset< Texture >( "textures.front_normal" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "metallicMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "roughMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "aoMap", mAssetManager->GetAsset< Texture >( "textures.white" ) );
		yellowMat.Get()->ConstCast< Material >()->SetUniform( "emissiveMap", mAssetManager->GetAsset< Texture >( "textures.black" ) );

		axisBoxMat->Save( );
		redMat->Save( );
		greenMat->Save( );
		blueMat->Save( );
		yellowMat->Save( ); 
	}
}


/*

	// Constructing new editor window ideas

	// Example - shader graph editor window

	class ShaderGraphEditorWindow : public EditorWindow
	{
		public:

			ShaderGraphEditorWindow()
			{
				// How should this work? Should it have a world with it? How would the user be able to do something like this? 
				// Should the material window even KNOW about the world it's using? 

				// Need to grab a scene ( how should this work, in general? )
				
				World* world = GetWorld();

				// Where are worlds stored? How are they created / accessed / destroyed and cleaned up? 
				// Have a concern about doing tons of post processing for multiple scenes as well
				// Might need to look into a job system sooner than later

				// The shader graph editor window has multiple docked tabs ( I need a good way to associate myself with these; right now, there's no real way to do this... )
				// Properties view
				// Viewport
				// Node Graph Canvas

				// The viewport has a scene associated with it ( it either needs to create a new scene, or it needs to grab a scene from an existing viewport )
				// The viewport docked tab creates a new scene by default? I don't know if I like this, to be honest; it seems odd;
				// Do viewports hold graphics scenes? That seems fucking weird. It would seem more logical that a viewport has the option to simply be used for the rendering of any scene available. 
			}

		protected:
			StaticMeshRenderable mRenderable;
			AssetHandle< ShaderGraph > mCurrentGraph; 
	}; 

	class EntityContext
	{
		protected:
			Vector< EntityHandle > mActiveEntities;
	};

	EntityContext EntityManager::ConstructContext( World* world )
	{
		EntityContext ctx; 
	}

	ENJON_CLASS( )
	class World : public Object
	{
		ENJON_CLASS_BODY( World )

		public:

			World()
			{
				// Construct contexts
				mEntityContext = EngineSubsystem( EntityManager )->ConstructContext( this );
				mPhysicsContext = EngineSubsytem( PhysicsSubsystem )->ConstructContext( this );
				mGraphicsContext = EngineSubsystem( GraphicsSubsystem )->ConstructContext( this ); 
			}

		protected: 
			EntityContext mEntityContext;
			PhysicsContext mPhysicsContext;
			GraphicsContext mGraphicsContext;	

		private:
	};

	class EditorWindow : public Window
	{
		public:

			void Init()
			{
				// Call init to the base window class to initialize this window
				Window::Init();			
				GUIContext* guiContext = GetGUIContext( );
				assert( guiContext->GetContext( ) != nullptr );

				static bool sceneSelectionViewOpen = true;
				guiContext->RegisterWindow( "Scene Selection", [ & ]
				{
					if ( ImGui::BeginDock( "Scene Selection", &sceneSelectionViewOpen ) )
					{
						SelectSceneView( );
					}
					ImGui::EndDock( );
				} ); 
 
				guiContext->RegisterDockingLayout( GUIDockingLayout( "Scene Selection", nullptr, GUIDockSlotType::Slot_Tab, 0.2f ) ); 
				guiContext->RegisterDockingLayout( GUIDockingLayout( "Asset Browser", "Scene Selection", GUIDockSlotType::Slot_Top, 0.5f ) ); 
			}

		protected:

	};

	class EditorViewportView : public EditorView
	{
		public:

			EditorViewportView( EditorApp* app, Window* window, const String& name = "Viewport" )
				: EditorView( app, window, "Viewport", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
			{ 
				// Construct new viewport and grab handle from graphics subsystem
				mViewport = EngineSubsystem( GraphicsSubsystem )->ConstructViewport(); 

				mViewportCamera = Camera( iVec2( mViewport.GetDimensions() );
				mViewportCamera.SetNearFar( 0.1f, 1000.0f );
				mViewportCamera.SetProjection(ProjectionType::Perspective);
				mViewportCamera.SetPosition(Vec3(0, 5, 10)); 

				// Where do the scenes live? Does a world own a scene? I think that's a decent compromise and setup;
 
				mGfx->GetGraphicsScene( )->AddCamera( &mEditorCamera );
				mGfx->GetGraphicsScene()->SetActiveCamera( &mEditorCamera );
			}

		protected:

			String mName = "Viewport";

		private: 
			Viewport mViewport;
			Camera mViewportCamera;
	}; 

	ViewportHandle GraphicsSubsystem::ConstructViewport( const u32& width, const u32& height )
	{
		Viewport viewport;
		viewport.mWidth = width;
		viewport.mHeight = height;
		viewport.mRendertargetHandle = Construct< RenderTarget >(); 

		return viewport;
	}

	// Docks should be considered as "child windows", which means that they can only be docked with their own parent windows and can only have other docks dock with them that share the same parent window 

	// Should viewports be controlled by cameras? So you call - Viewport.SetCamera( Camera* cam ) ? And this is what the viewport uses itself? I don't think that cameras themselves should necessary be responsible for holding
	// Viewports, since a camera can be swapped around between views easily. 
	// Currently, the user gets a graphics scene then sets its active camera ( could do the same with a viewport );

	class Viewport
	{
		friend GraphicsSubsystem;

		public:
			
			iVec2 GetDimensions()
			{
				return iVec2( mWidth, mHeight );
			}

		protected: 
			u32 mWidth;
			u32 mHeight;
			GraphicsHandle< RenderTarget > mRendertargetHandle; 
	};
	




















*/


