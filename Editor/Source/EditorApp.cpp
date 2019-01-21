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
#include <Physics/CollisionShape.h>
#include <Entity/Components/StaticMeshComponent.h>
#include <Entity/Components/RigidBodyComponent.h>
#include <Entity/Components/PointLightComponent.h>
#include <Entity/Components/DirectionalLightComponent.h>
#include <Utils/FileUtils.h> 
#include <Utils/Tokenizer.h> 

#include <Base/World.h> 

#include <windows.h>
#include <fmt/format.h>
#include <chrono>
#include <ctime>

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

Enjon::String projectName = "TestProject";
Enjon::String projectDLLName = projectName + ".dll";
Enjon::String copyDir = ""; 
Enjon::String mProjectsDir = "E:/Development/EnjonProjects/";
Enjon::String mVisualStudioDir = "\"E:\\Programs\\MicrosoftVisualStudio14.0\\\"";

//Enjon::String configuration = "Release";
//Enjon::String configuration = "RelWithDebInfo";
Enjon::String configuration = "Debug";

namespace Enjon
{
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

		// Save scene
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

			ImGui::SameLine( );
			if ( ImGui::Button( "Reload" ) )
			{ 
				// Reload the dll
				ReloadDLL( );
			}

			// Compile the project if available
			if ( mProject.GetApplication( ) )
			{
				ImGui::SameLine( ); 
				if ( ImGui::Button( "Compile" ) )
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
		String projectDir = mProjectsDir + projectName + "/";
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

	bool EditorApp::CreateProjectView( )
	{
		char buffer[ 256 ];
		strncpy( buffer, mNewProjectName.c_str( ), 256 );
		if ( ImGui::InputText( "Project Name", buffer, 256 ) )
		{
			mNewProjectName = String( buffer );
		}

		if ( ImGui::Button( "Create New Project" ) && mNewProjectName.compare( "" ) != 0 )
		{
			// If project is able to be made, then make it
			String projectPath = mProjectsDir + "/" + mNewProjectName + "/";
			if ( !fs::exists( projectPath ) )
			{
				//CreateNewProject( mNewProjectName );
				PreCreateNewProject( mNewProjectName );
				return true;
			}
			else
			{
				std::cout << "Project already exists!\n";
			} 
		}

		return false;

		//if ( !mPlaying )
		//{ 
		//	// Load visual studio project 
		//	if ( ImGui::Button( "Load Project Solution" ) )
		//	{
		//		if ( fs::exists( mProject.GetProjectPath( ) + "Build/" + mProject.GetProjectName( ) + ".sln" ) )
		//		{ 
		//			LoadProjectSolution( );
		//		}
		//	}
		//}

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

	void EditorApp::CollectAllProjectsOnDisk( )
	{ 
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

		mainWindow->SetSize( iVec2( 1400, 900 ) ); 
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

		auto createViewOption = [&]()
		{
			if ( mProject.GetApplication() && !mPlaying )
			{
				if ( ImGui::MenuItem( "Empty##options", NULL ) )
				{
					std::cout << "Creating empty entity...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					EntityHandle empty = em->Allocate( );
					empty.Get( )->SetName( "Empty" );

					// Set to selected entity
					mWorldOutlinerView->SelectEntity( empty );
				}

				if ( ImGui::MenuItem( "CubeIComponent##options", NULL ) )
				{
					std::cout << "Creating IComponent cube" << "\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );

					EntityHandle cube = em->Allocate( );
					if ( cube )
					{
						Entity* ent = cube.Get( );
						GraphicsScene* gs = ent->GetWorld( )->ConstCast< World >( )->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( );
						ComponentHandle< StaticMeshComponent > ch = em->AddComponent< StaticMeshComponent >( ent );
						ch->mRenderableHandle->SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );

						const Camera* cam = gfx->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f ); 

						ent->SetName( "Cube" );

						// Select entity
						mWorldOutlinerView->SelectEntity( cube ); 
					}
				}

				if ( ImGui::MenuItem( "Cube##options", NULL ) )
				{
					std::cout << "Creating cube...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle cube = em->Allocate( );
					if ( cube )
					{
						Entity* ent = cube.Get( );
						ComponentHandle< StaticMeshComponent > gfx = ent->AddComponent<StaticMeshComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ), 0 );

						ComponentHandle< RigidBodyComponent > rbc = ent->AddComponent<RigidBodyComponent>( );
						rbc->SetShape( CollisionShapeType::Box ); 

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );

						ent->SetName( "Cube" );
					}

					// Select entity
					mWorldOutlinerView->SelectEntity( cube );
				}

				if ( ImGui::MenuItem( "Sphere##options", NULL ) )
				{
					std::cout << "Creating sphere...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					AssetManager* am = EngineSubsystem( AssetManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle sphere = em->Allocate( );
					if ( sphere )
					{
						Entity* ent = sphere.Get( );
						ComponentHandle< StaticMeshComponent > gfx = ent->AddComponent<StaticMeshComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_sphere" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ), 0 );

						ComponentHandle< RigidBodyComponent > rbc = ent->AddComponent< RigidBodyComponent >( );
						rbc->SetShape( CollisionShapeType::Sphere );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );

						ent->SetName( "Sphere" );
					}

					// Select entity
					mWorldOutlinerView->SelectEntity( sphere );
				}

				if ( ImGui::MenuItem( "Point Light##options", NULL ) )
				{
					std::cout << "Creating point light...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					GraphicsSubsystem* gs = EngineSubsystem( GraphicsSubsystem );
					EntityHandle pointLight = em->Allocate( );
					if ( pointLight )
					{
						Entity* ent = pointLight.Get( ); 
						ent->AddComponent<PointLightComponent>( );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f ); 
						ent->SetName( "PointLight" );
					}

					// Select entity
					mWorldOutlinerView->SelectEntity( pointLight );
				}

				if ( ImGui::MenuItem( "Directional Light##options", NULL ) )
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
					mWorldOutlinerView->SelectEntity( directionalLight );
				}

				if ( ImGui::MenuItem( "Scene##options", NULL ) )
				{
					// Construct scene and save it
					AssetManager* am = EngineSubsystem( AssetManager );
					AssetHandle< Scene > scene = am->ConstructAsset< Scene >( );

					// Unload scene after saving it
					// NOTE(): Shouldn't have to do this
					scene.Unload( ); 
				} 

				if ( ImGui::MenuItem( "Material##Options", NULL ) )
				{
					// Construct material asset and save it
					AssetManager* am = EngineSubsystem( AssetManager );

					// Don't name it for now
					AssetHandle< Material > mat = am->ConstructAsset< Material >( ); 
				}
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
		};

		// Register menu options
		//guiContext->RegisterMenuOption("File", "Load Project...##options", loadProjectMenuOption); 
		guiContext->RegisterMenuOption("File", "Save Scene##options", saveSceneOption); 
		guiContext->RegisterMenuOption( "Create", "Create", createViewOption );

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

	void EditorApp::LoadProjectSelectionContext( )
	{
		// Set the window to small
		Window* window = EngineSubsystem( GraphicsSubsystem )->GetMainWindow( );
		assert( window != nullptr );

		// Set the size of the window
		window->HideWindow( );

		mProjectSelectionWindow = new Window( );
		mProjectSelectionWindow->Init( "Test", 1200, 500, WindowFlags::DEFAULT );
		mProjectSelectionWindow->SetWindowTitle( "Enjon Editor: Project Selection / Creation" ); 
		EngineSubsystem( GraphicsSubsystem )->AddWindow( mProjectSelectionWindow ); 

		GUIContext* guiCtx = mProjectSelectionWindow->GetGUIContext( );

		auto createProjectView = [ & ] ( )
		{ 
			if ( ImGui::BeginDock( "Project Selection / Creation", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize ) )
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
							Window::DestroyWindow( mProjectSelectionWindow );
						}
					}
					ImGui::EndCombo( );
				} 

				// Create project view underneath this
				ImGui::NewLine( );

				if ( CreateProjectView( ) )
				{
					Window::DestroyWindow( mProjectSelectionWindow );
				}
			}
			ImGui::EndDock( );
		}; 

		guiCtx->RegisterWindow( "Project Selection / Creation", createProjectView ); 
		guiCtx->RegisterDockingLayout( GUIDockingLayout( "Project Selection / Creation", nullptr, GUIDockSlotType::Slot_Top, 1.0f ) ); 

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

		// Grab all .eproj files and store them for loading later
		CollectAllProjectsOnDisk( ); 

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


