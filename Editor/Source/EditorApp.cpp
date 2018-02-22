#include "EditorApp.h"
#include "EditorSceneView.h"

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
#include <Entity/Components/GraphicsComponent.h>
#include <Entity/Components/RigidBodyComponent.h>
#include <Utils/FileUtils.h>

#include <Bullet/btBulletDynamicsCommon.h> 

#include <windows.h>
#include <fmt/format.h>
#include <chrono>
#include <ctime>

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
//Enjon::String mProjectsDir = "E:/Development/EnjonProjects/";
Enjon::String mProjectsDir = "W:/Projects/";

Enjon::String configuration = "Release";
//Enjon::String configuration = "RelWithDebInfo";
//Enjon::String configuration = "Debug";

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

	void EditorApp::AddComponentPopupView( )
	{
		if ( ImGui::BeginPopupModal( "##NewComponent" ) )
		{
			// Get component list
			EntityManager* entities = EngineSubsystem( EntityManager );
			auto compMetaClsList = entities->GetComponentMetaClassList( );

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
			if ( ImGui::InputText( "Component Name", buffer, 256 ) )
			{
				// Reset component name
				componentName = String(buffer);
			} 

			if ( ImGui::Button( "Create" ) )
			{
				if ( !alreadyExists( componentName ) )
				{
					closePopup = true;
					std::cout << "Creating component!\n";
				}
				else
				{
					std::cout << "Component already exists!\n";
				}
			}

			if ( ImGui::Button( "Cancel" ) )
			{
				closePopup = true;
			}

			if ( closePopup )
			{
				ImGui::CloseCurrentPopup( );
				mNewComponentPopupDialogue = false; 
			}

			ImGui::EndPopup( );
		}
	}

	void EditorApp::InspectorView( bool* enabled )
	{
		if ( mSelectedEntity )
		{
			// Debug dump the entity ( Probably shouldn't do this and should tailor it more... )
			Entity* ent = mSelectedEntity.Get( ); 
			//ImGuiManager::DebugDumpObject( ent );

			// Transform information
			if ( ImGui::CollapsingHeader( "Local Transform" ) )
			{
				ImGuiManager::DebugDumpProperty( ent, ent->Class( )->GetPropertyByName( "mLocalTransform" ) ); 
			}

			if ( ImGui::BeginCombo( "##ADDCOMPONENT", "Add Component..." ) )
			{
				// Get component list
				EntityManager* entities = EngineSubsystem( EntityManager );
				auto compMetaClsList = entities->GetComponentMetaClassList( );

				for ( auto& cls : compMetaClsList )
				{
					if ( !ent->HasComponent( cls ) )
					{
						// Add component to mEntity
						if ( ImGui::Selectable( cls->GetName( ).c_str( ) ) )
						{
							ent->AddComponent( cls );
						} 
					}
				}

				if ( ImGui::Selectable( "New Component..." ) )
				{
					mNewComponentPopupDialogue = true;
				}

				ImGui::EndCombo( );
			}

			// Add new component dialogue window
			if ( mNewComponentPopupDialogue )
			{
				ImGui::OpenPopup( "##NewComponent" );
				AddComponentPopupView( );
			}

			for ( auto& c : ent->GetComponents( ) )
			{
				if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
				{
					MetaClass* cls = const_cast< MetaClass * > ( c->Class( ) );
					s32 shapeType = -1;
					if ( cls->InstanceOf<RigidBodyComponent>( ) )
					{
						shapeType = (s32)c->Cast<RigidBodyComponent>( )->GetShapeType( );
					}

					ImGuiManager::DebugDumpObject( c ); 

					// Shape type changed
					// TODO(): Be able to hook in specific delegates for property changes through reflection system
					if ( shapeType != -1 && shapeType != (s32)c->Cast< RigidBodyComponent >()->GetShapeType() )
					{
						c->ConstCast< RigidBodyComponent >( )->SetShape( c->Cast< RigidBodyComponent >( )->GetShapeType( ) );
					} 

					if ( ImGui::Button( "Remove" ) )
					{
						ent->RemoveComponent( c->Class( ) );
					}
				}
			}
		}
	}

	//===========================================================================================

	void EditorWidgetManager::Finalize( )
	{
		// Register windows with ImGuiManager
		for ( auto& v : mViews ) 
		{ 
			// Register individual windows
			Enjon::ImGuiManager::RegisterWindow( [ & ] ( )
			{
				// Docking windows
				if ( ImGui::BeginDock( v->GetViewName().c_str(), &mViewEnabledMap[v], v->GetViewFlags() ) )
				{
					v->Update( );
				}
				ImGui::EndDock( ); 
			}); 
		}
	}

	//===========================================================================================

	EditorWidgetManager* EditorApp::GetEditorWidgetManager( )
	{
		return &mEditorWidgetManager;
	}

	//===========================================================================================

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
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		const Enjon::Camera* cam = gfx->GetGraphicsSceneCamera( );

		if ( ImGui::TreeNode( "Camera" ) )
		{
			Enjon::ImGuiManager::DebugDumpObject( cam ); 
			ImGui::TreePop( );
		}

		ImGui::DragFloat( "Camera Speed", &mCameraSpeed, 0.01f, 0.01f, 100.0f ); 
		ImGui::DragFloat( "Mouse Sensitivity", &mMouseSensitivity, 0.1f, 0.1f, 10.0f ); 

		if ( ImGui::TreeNode( "Application" ) )
		{
			if ( mProject.GetApplication() )
			{
				Enjon::ImGuiManager::DebugDumpObject( mProject.GetApplication() ); 
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
				cam->SetPosition( mPreviousCameraTransform.Position );
				cam->SetRotation( mPreviousCameraTransform.Rotation ); 
			}

			static bool isPaused = false;

			if ( !isPaused )
			{
				ImGui::SameLine( );
				if ( ImGui::Button( "Pause" ) )
				{
					isPaused = true;
					PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
					physx->PauseSystem( true );
				} 
			} 
			else
			{
				ImGui::SameLine( );
				if ( ImGui::Button( "Resume" ) )
				{
					isPaused = false;
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
				// ReloadDLL without release scene asset
				LoadDLL( false );
			}
		} 
	}

	void EditorApp::SelectEntity( const EntityHandle& handle )
	{
		mSelectedEntity = handle;

		// Enable transform widget
		mTransformWidget.Enable( true );

		// Set transform to selected entity
		mTransformWidget.SetRotation( mSelectedEntity.Get( )->GetWorldRotation( ) );
		mTransformWidget.SetPosition( mSelectedEntity.Get( )->GetWorldPosition( ) );
	}

	void EditorApp::DeselectEntity( )
	{
		// Set to invalid entity handle
		mSelectedEntity = EntityHandle::Invalid();

		// Deactivate transform widget
		mTransformWidget.Enable( false );
	}

	void EditorApp::WorldOutlinerView( )
	{ 
		if ( !mCurrentScene )
		{
			return;
		}

		EntityManager* entities = EngineSubsystem( EntityManager );

		// Print out scene name
		ImGui::Text( ( "Scene: " + mCurrentScene->GetName( ) ).c_str( ) );

		ImGui::Separator( );

		// List out active entities
		for ( auto& e : entities->GetActiveEntities( ) )
		{
			if ( ImGui::Selectable( fmt::format( "{}", e->GetID() ).c_str( ) ) )
			{
				SelectEntity( e );
			} 
		}
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
		String defaultText = mCurrentScene.Get( ) == nullptr ? "Available Scenes..." : mCurrentScene->GetName( );
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
						// Unload all current entities in scene ( This is something that a scene manager would likely handle )
						UnloadScene( );

						// Load the asset from disk
						record.second.LoadAsset( ); 

						// Set current scene with record's asset
						mCurrentScene = record.second.GetAsset();

						// Deselect entity
						DeselectEntity( );
					} 
				} 
			}
			else
			{
				ImGui::Text( "No project loaded." );
			}

			ImGui::EndCombo( ); 
		}

		if ( mCurrentScene )
		{
			if ( ImGui::Button( "Save Scene" ) )
			{
				mCurrentScene->Save( );
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
		Enjon::Utils::WriteToFile( "", projectDir + "Build/Generator/Linked/" + projectName + "_Generated.cpp" ); 
		Enjon::Utils::WriteToFile( projectDir, projectDir + projectName + ".eproj" );

		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Error check the fuck out of this call
		// Is it possible to know whether or not this succeeded?
		s32 code = system( String( "start " + projectDir + "Proc/" + "BuildAndRun.bat" + " " + Enjon::Utils::FindReplaceAll( projectDir, "/", "\\" ) + " " + projectName ).c_str() ); 
		if ( code == 0 )
		{
			// Can set the project path now
			mProject.SetProjectPath( projectDir ); 
			mProject.SetProjectName( projectName );

			// Unload previous project
			UnloadDLL( ); 
		}
#endif

		// Add project to disk
		Project proj; 
		proj.SetProjectPath( projectDir );
		proj.SetProjectName( projectName );
		mProjectsOnDisk.push_back( proj ); 
	}

	//================================================================================================================================

	void EditorApp::UnloadScene( bool releaseSceneAsset )
	{
		EntityManager* em = EngineSubsystem( EntityManager );

		// Destroy all entities that are pending to be added as well
		em->DestroyAll( );

		// Force cleanup of scene
		em->ForceCleanup( );

		if ( releaseSceneAsset )
		{
			if ( mCurrentScene )
			{
				mCurrentScene.Unload( ); 
			}

			mCurrentScene = nullptr; 
		}
	}

	//================================================================================================================================

	void EditorApp::LoadProject( const Project& project )
	{ 
		// TODO(): Unload current project - which means to destroy all current entities
		UnloadScene( );

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

	void EditorApp::CreateProjectView( )
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
				CreateNewProject( mNewProjectName );
			}
			else
			{
				std::cout << "Project already exists!\n";
			} 
		}

		if ( !mPlaying )
		{ 
			String defaultText = mProject.GetApplication( ) == nullptr ? "Existing Projects..." : mProject.GetProjectName( );
			if ( ImGui::BeginCombo( "##LOADPROJECT", defaultText.c_str() ) )
			{
				for ( auto& p : mProjectsOnDisk )
				{
					if ( ImGui::Selectable( p.GetProjectName( ).c_str( ) ) )
					{ 
						// Load the project
						LoadProject( p );
					}
				}

				ImGui::EndCombo( );
			} 

			// Load visual studio project 
			if ( ImGui::Button( "Load Project Solution" ) )
			{
				if ( fs::exists( mProject.GetProjectPath( ) + "Build/" + mProject.GetProjectName( ) + ".sln" ) )
				{ 
					LoadProjectSolution( );
				}
			}
		}

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

	void EditorApp::LoadDLL( bool releaseSceneAsset )
	{
		Enjon::ByteBuffer buffer;

		// NOTE(): VERY SPECIFIC IMPL THAT WILL BE GENERALIZED TO RELOADING SCENE
		// Serialize mEntity ( this will be the scene, essentially... )
		//EntityArchiver::Serialize( mSceneEntity, &buffer );

		// Destroy scene entity
		//mSceneEntity.Get()->Destroy( );

		// Clean mSceneEntities
		//mSceneEntities.clear( );

		// Force the scene to clean up ahead of frame
		//CleanupScene( ); 

		// Unload previous scene 
		UnloadScene( releaseSceneAsset ); 

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

		// Reload scene if available
		if ( !releaseSceneAsset )
		{
			if ( mCurrentScene )
			{
				mCurrentScene.Reload( ); 
			}
		}
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
				mProjectsOnDisk.push_back( proj );
			}
		} 
	}

	//================================================================================================================================

	void EditorApp::ReloadScene( )
	{
		if ( mCurrentScene )
		{
			mCurrentScene.Reload( );
		}
	}

	//================================================================================================================================

	void EditorApp::InitializeProjectApp( )
	{
		Application* app = mProject.GetApplication( );
		if ( app && mCurrentScene )
		{
			// Set application state to running
			SetApplicationState( ApplicationState::Running );

			// Cache off all entity handles in scene before app starts
			EntityManager* em = EngineSubsystem( EntityManager );
			mSceneEntities = em->GetActiveEntities( );

			// NOTE(): Don't really like this at all...
			// Save current scene before starting so we can reload on stop 
			mCurrentScene.Save( );

			// Initialize the app
			app->Initialize( );

			// Run through all components for entities and call their initialize function
			for ( auto& e : em->GetActiveEntities( ) )
			{
				for ( auto& c : e->GetComponents( ) )
				{
					c->Initialize( );
				}
			}

			// Run through all components for entities and call their start function
			for ( auto& e : em->GetActiveEntities( ) )
			{
				for ( auto& c : e->GetComponents( ) )
				{
					c->Start( );
				}
			}

			// Turn on the physics simulation
			PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
			physx->PauseSystem( false ); 
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
			Vector<Entity*> entities = em->GetActiveEntities( );
			// Destroy any entities alive that aren't in the cached off entity list
			for ( auto& e : entities )
			{ 
				// Shutdown its components
				for ( auto& c : e->GetComponents( ) )
				{
					c->Shutdown( );
				}

				// Destroy all entities
				// If either null or not in original cached entity list then destroy
				//if ( !e || std::find( mSceneEntities.begin( ), mSceneEntities.end( ), e ) == mSceneEntities.end( ) )
				//{
				//	e->Destroy( );
				//}
			} 

			// Destroy all entities
			em->DestroyAll( );

			// Shutodwn the application
			app->Shutdown( ); 

			// Force the scene to clean up ahead of frame
			CleanupScene( ); 

			// Reload current scene
			ReloadScene( );

			// Clean up physics subsystem from contact events as well
			PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
			phys->Reset( );

			// Pause the physics simulation
			phys->PauseSystem( true ); 
		}
	}
	 
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

		// Pause the physics simulation
		physx->PauseSystem( true ); 

		// Register project template files
		mProjectSourceTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectSourceTemplate.cpp" ).c_str() ); 
		mProjectCMakeTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectCMakeTemplate.txt" ).c_str( ) );
		mProjectDelBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/DelPDB.bat" ).c_str( ) );
		mProjectBuildAndRunTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/BuildAndRun.bat" ).c_str( ) ); 

		// Set up copy directory for project dll
		copyDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + projectName + "/";

		// Grab all .eproj files and store them for loading later
		CollectAllProjectsOnDisk( ); 

		// Add all necessary views into editor widget manager
		mEditorWidgetManager.AddView( new EditorSceneView( this ) );

		// Initialize editor widget manager
		mEditorWidgetManager.Finalize( );

		// Initialize transform widget
		mTransformWidget.Initialize( this ); 

		// Register individual windows
		Enjon::ImGuiManager::RegisterWindow( [ & ] ( )
		{
			// Docking windows
			if ( ImGui::BeginDock( "Create Project", &mShowCreateProjectView, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
			{
				// Print docking information
				CreateProjectView( );
			}
			ImGui::EndDock( );
		} );

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Camera", &mShowCameraOptions ) )
			{
				CameraOptions( &mShowCameraOptions );
			}
			ImGui::EndDock( );
		});

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Load Resource", &mShowLoadResourceOption ) )
			{
				LoadResourceFromFile( );
			}
			ImGui::EndDock( );
		});

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "World Outliner", nullptr ) )
			{
				WorldOutlinerView( );
			}
			ImGui::EndDock( );
		});

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Play Options", nullptr ) )
			{
				PlayOptions( );
			}
			ImGui::EndDock( );
		} );

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Inspector View", nullptr ) )
			{
				InspectorView( nullptr );
			}
			ImGui::EndDock( );
		} );

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Scene Selection View", nullptr ) )
			{
				SelectSceneView( );
			}
			ImGui::EndDock( );
		} );

		mShowSceneView = true;
		auto sceneViewOption = [ & ] ( )
		{
			ImGui::MenuItem( "Scene##options", NULL, &mShowSceneView );
		};

		auto createViewOption = [&]()
		{
			if ( mProject.GetApplication() && !mPlaying )
			{
				if ( ImGui::MenuItem( "Empty##options", NULL ) )
				{
					std::cout << "Creating empty entity...\n";
					EntityManager* em = EngineSubsystem( EntityManager );
					EntityHandle empty = em->Allocate( );

					// Set to selected entity
					SelectEntity( empty );
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
						GraphicsComponent* gfx = ent->AddComponent<GraphicsComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ) );

						RigidBodyComponent* rbc = ent->AddComponent<RigidBodyComponent>( );
						rbc->SetShape( CollisionShapeType::Box ); 

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
					}

					// Select entity
					SelectEntity( cube );
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
						GraphicsComponent* gfx = ent->AddComponent<GraphicsComponent>( );
						gfx->SetMesh( am->GetAsset< Mesh >( "models.unit_sphere" ) );
						gfx->SetMaterial( am->GetDefaultAsset<Material>( ) );

						RigidBodyComponent* rbc = ent->AddComponent< RigidBodyComponent >( );
						rbc->SetShape( CollisionShapeType::Sphere );

						const Camera* cam = gs->GetGraphicsSceneCamera( );
						ent->SetLocalPosition( cam->GetPosition( ) + cam->Forward( ) * 5.0f );
					}

					// Select entity
					SelectEntity( sphere );
				}

				if ( ImGui::MenuItem( "Scene##options", NULL ) )
				{
					// Construct scene and save it
					AssetManager* am = EngineSubsystem( AssetManager );
					AssetHandle< Scene > scene = am->ConstructAsset< Scene >( );

					// Unload scene after saving it
					scene.Unload( ); 
				} 
			}
		}; 

		// Register menu options
		ImGuiManager::RegisterMenuOption( "Create", createViewOption );

		// Register docking layouts 
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene View", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Camera", "Scene View", ImGui::DockSlotType::Slot_Right, 0.2f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Load Resource", "Camera", ImGui::DockSlotType::Slot_Bottom, 0.3f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "World Outliner", "Camera", ImGui::DockSlotType::Slot_Top, 0.7f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Play Options", "Scene View", ImGui::DockSlotType::Slot_Top, 0.1f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Create Project", "Play Options", ImGui::DockSlotType::Slot_Tab, 0.1f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Inspector View", "World Outliner", ImGui::DockSlotType::Slot_Bottom, 0.5f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene Selection View", "World Outliner", ImGui::DockSlotType::Slot_Bottom, 0.5f ) );

		return Enjon::Result::SUCCESS;
	}

	//=================================================================================================

	void EditorApp::SetEditorSceneView( EditorSceneView* view )
	{
		mEditorSceneView = view;
	}

	//=================================================================================================

	Enjon::Result EditorApp::Update( f32 dt )
	{ 
		// Update transform widget
		mTransformWidget.Update( );

		// Simulate game tick scenario if playing
		if ( mPlaying )
		{
			Application* app = mProject.GetApplication( );
			if ( app )
			{
				//static Enjon::ByteBuffer temp;
				//Enjon::ObjectArchiver::Serialize( app, &temp );

				app->ProcessInput( dt );
				app->Update( dt );
			}
		}

		return Enjon::Result::PROCESS_RUNNING;
	}

	enum class TransformMode
	{
		Translate,
		Scale
	};

	Enjon::Result EditorApp::ProcessInput( f32 dt )
	{
		static bool mInteractingWithTransformWidget = false;
		static TransformMode mMode = TransformMode::Translate;
		static Vec3 mIntersectionStartPosition;
		static Vec3 mRootStartPosition;
		static Vec3 mRootStartScale;
		static TransformWidgetRenderableType mType;
		static GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		static Vec2 mMouseCoordsDelta = Vec2( 0.0f );
		Input* mInput = EngineSubsystem( Input );
		Camera* camera = mGfx->GetGraphicsSceneCamera( )->ConstCast< Enjon::Camera >( );
		Enjon::iVec2 viewPort = mGfx->GetViewport( ); 
		Enjon::Window* window = mGfx->GetWindow( )->ConstCast< Enjon::Window >( );

		// Can move camera if scene view has focus
		bool previousCamMove = mMoveCamera;
		mMoveCamera = mEditorWidgetManager.GetFocused( mEditorSceneView );
		if ( mMoveCamera && ( mMoveCamera != previousCamMove ) )
		{ 
			Vec2 mc = mInput->GetMouseCoords( );
			Vec2 center = mEditorSceneView->GetCenterOfViewport( );
			mMouseCoordsDelta = Vec2( (f32)(viewPort.x) / 2.0f - mc.x, (f32)(viewPort.y) / 2.0f - mc.y );
		}

		if ( !mMoveCamera )
		{
			if ( mInput->IsKeyPressed( KeyCode::Delete ) )
			{
				if ( mSelectedEntity )
				{
					mSelectedEntity.Get( )->Destroy( );
					DeselectEntity( );
				}
			}

			if ( mSelectedEntity )
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
			}

			if ( mInput->IsKeyDown( KeyCode::LeftMouseButton ) )
			{
				if ( mTransformWidget.IsInteractingWithWidget( ) )
				{
					mTransformWidget.InteractWithWidget( );
					Vec3 delta = mTransformWidget.GetDelta( );

					TransformationMode transformationMode = mTransformWidget.GetTransformationMode( );

					switch ( transformationMode )
					{
						case TransformationMode::Translation:
						{
							switch ( mTransformWidget.GetInteractedWidgetType( ) )
							{
								case TransformWidgetRenderableType::TranslationXYAxes:
								case TransformWidgetRenderableType::TranslationXZAxes:
								case TransformWidgetRenderableType::TranslationYZAxes:
								case TransformWidgetRenderableType::TranslationRoot:
								case TransformWidgetRenderableType::TranslationUpAxis:
								case TransformWidgetRenderableType::TranslationForwardAxis:
								case TransformWidgetRenderableType::TranslationRightAxis:
								{
									Entity* ent = mSelectedEntity.Get( );
									if ( ent )
									{
										Vec3 lp = ent->GetLocalPosition( ) + delta;
										ent->SetLocalPosition( lp );
									}
								} break;
							}

						} break;

						case TransformationMode::Scale:
						{
							switch ( mTransformWidget.GetInteractedWidgetType( ) )
							{
								case TransformWidgetRenderableType::ScaleYZAxes:
								case TransformWidgetRenderableType::ScaleXZAxes:
								case TransformWidgetRenderableType::ScaleXYAxes:
								case TransformWidgetRenderableType::ScaleForwardAxis:
								case TransformWidgetRenderableType::ScaleUpAxis:
								case TransformWidgetRenderableType::ScaleRightAxis:
								{
									Entity* ent = mSelectedEntity.Get( );
									if ( ent )
									{
										Vec3 ls = ent->GetLocalScale( ) + delta;
										ent->SetLocalScale( ls );
									}
								} break;
								case TransformWidgetRenderableType::ScaleRoot:
								{
									Entity* ent = mSelectedEntity.Get( );
									if ( ent )
									{
										Vec3 ls = ent->GetLocalScale( ) + delta * ent->GetLocalScale( );
										ent->SetLocalScale( ls );
									}

								} break;
							}

						} break;

						case TransformationMode::Rotation:
						{ 
							Entity* ent = mSelectedEntity.Get( );
							if ( ent )
							{
								ent->SetLocalRotation( ent->GetLocalRotation() * mTransformWidget.GetDeltaRotation() );
								//ent->SetLocalRotation( mTransformWidget.GetDeltaRotation() );
							}
						} break;
					}

					if ( mSelectedEntity.Get( ) )
					{
						Entity* ent = mSelectedEntity.Get( );
						// Set position and rotation to that of entity
						mTransformWidget.SetPosition( ent->GetWorldPosition( ) );
						mTransformWidget.SetRotation( ent->GetWorldRotation( ) );
					}
				} 
			}
			else
			{
				mTransformWidget.EndInteraction( );
			}

		}

		if ( mMoveCamera )
		{ 
			Enjon::Vec3 velDir( 0, 0, 0 );

			if ( mInput->IsKeyDown( Enjon::KeyCode::W ) )
			{
				Enjon::Vec3 F = camera->Forward( );
				velDir += F;
			}
			if ( mInput->IsKeyDown( Enjon::KeyCode::S ) )
			{
				Enjon::Vec3 B = camera->Backward( );
				velDir += B;
			}
			if ( mInput->IsKeyDown( Enjon::KeyCode::A ) )
			{
				velDir += camera->Left( );
			}
			if ( mInput->IsKeyDown( Enjon::KeyCode::D ) )
			{
				velDir += camera->Right( );
			}

			// Normalize velocity
			velDir = Enjon::Vec3::Normalize( velDir );

			f32 avgDT = Engine::GetInstance( )->GetWorldTime( ).GetDeltaTime( );

			// Set camera position
			camera->Transform.Position += mCameraSpeed * avgDT * velDir;

			// Set camera rotation
			// Get mouse input and change orientation of camera
			Enjon::Vec2 mouseCoords = mInput->GetMouseCoords( );

			//Vec2 mouseCoords = mEditorSceneView->GetSceneViewProjectedCursorPosition( );

			// Set cursor to not visible
			window->ShowMouseCursor( false );

			// Reset the mouse coords after having gotten the mouse coordinates
			Vec2 center = mEditorSceneView->GetCenterOfViewport( );
			//SDL_WarpMouseInWindow( window->GetWindowContext( ), (s32)center.x, (s32)center.y );
			SDL_WarpMouseInWindow( window->GetWindowContext( ), ( f32 )viewPort.x / 2.0f - mMouseCoordsDelta.x, ( f32 )viewPort.y / 2.0f - mMouseCoordsDelta.y );

			// Offset camera orientation
			f32 xOffset = Enjon::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x - mMouseCoordsDelta.x ) * avgDT * mMouseSensitivity;
			f32 yOffset = Enjon::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y - mMouseCoordsDelta.y ) * avgDT * mMouseSensitivity;
			camera->OffsetOrientation( xOffset, yOffset );
		}

		// Mouse cursor on, interact with world
		else
		{
			mGfx->GetWindow( )->ConstCast< Enjon::Window >( )->ShowMouseCursor( true );

			if ( mInput->IsKeyPressed( KeyCode::LeftMouseButton ) )
			{
				auto viewport = mGfx->GetViewport( );
				auto mp = GetSceneViewProjectedCursorPosition( );

				iVec2 dispSize = mGfx->GetViewport( );
				PickResult pr = mGfx->GetPickedObjectResult( GetSceneViewProjectedCursorPosition( ) );
				if ( pr.mEntity.Get( ) )
				{
					// Set selected entity
					SelectEntity( pr.mEntity );
				}
				// Translation widget interaction
				else if ( EditorTransformWidget::IsValidID( pr.mId ) )
				{
					// Begin widget interaction
					mTransformWidget.BeginWidgetInteraction( TransformWidgetRenderableType( pr.mId - MAX_ENTITIES ) );
				}
				else
				{
					// Deselect entity if click in scene view and not anything valid
					if ( mEditorWidgetManager.GetHovered( mEditorSceneView ) )
					{
						DeselectEntity( ); 
					}
				}
			}
		}

		if ( mInput->IsKeyPressed( KeyCode::Y ) )
		{
			Entity* ent = mSelectedEntity.Get( );
			if ( ent )
			{
				ent->SetLocalPosition( Vec3( 0.0f ) );
				ent->SetLocalRotation( Quaternion( ) );
			}

			mTransformWidget.SetPosition( Vec3( 0.0f ) );
			mTransformWidget.SetRotation( Quaternion( ) );
		}

		// Starting /Stopping game instance
		{ 
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

					camera->SetPosition( mPreviousCameraTransform.Position );
					camera->SetRotation( mPreviousCameraTransform.Rotation );
				}
			}
		}

		if ( !mTransformWidget.IsInteractingWithWidget( ) && mSelectedEntity )
		{
			mTransformWidget.SetPosition( mSelectedEntity.Get( )->GetWorldPosition( ) );
			mTransformWidget.SetRotation( mSelectedEntity.Get( )->GetWorldRotation( ) );
		}

		return Enjon::Result::PROCESS_RUNNING;
	}

	Enjon::Result EditorApp::Shutdown( )
	{ 
		return Enjon::Result::SUCCESS;
	} 

	void EditorApp::LoadResources( )
	{
		// Paths to resources
		Enjon::String toyBoxDispPath		= Enjon::String("Textures/toy_box_disp.png");
		Enjon::String toyBoxNormalPath		= Enjon::String("Textures/toy_box_normal.png");
		Enjon::String eyePath				= Enjon::String("Models/eye.obj");
		Enjon::String noisePath				= Enjon::String("Textures/worleyNoise.png");
		Enjon::String grassTexturePath		= Enjon::String("Textures/grass.png");
		Enjon::String quadPath				= Enjon::String("Models/quad.obj");
		Enjon::String rockPath				= Enjon::String("Models/rock.obj");
		Enjon::String cerebusMeshPath		= Enjon::String("Models/cerebus.obj");
		Enjon::String buddhaMeshPath		= Enjon::String("Models/buddha.obj");
		Enjon::String bunnyMeshPath			= Enjon::String("Models/bunny.obj");
		Enjon::String dragonMeshPath		= Enjon::String("Models/dragon.obj");
		Enjon::String monkeyMeshPath		= Enjon::String("Models/monkey.obj");
		Enjon::String sphereMeshPath		= Enjon::String("Models/unit_sphere.obj");
		Enjon::String cylinderMeshPath		= Enjon::String("Models/unit_cylinder.obj");
		Enjon::String coneMeshPath			= Enjon::String("Models/unit_cone.obj");
		Enjon::String cubeMeshPath			= Enjon::String("Models/unit_cube.obj");
		Enjon::String shaderballPath		= Enjon::String("Models/shaderball.obj");
		Enjon::String unitSpherePath		= Enjon::String("Models/unit_sphere.obj");
		Enjon::String unrealShaderBallPath	= Enjon::String("Models/unreal_shaderball.obj");
		Enjon::String unitShaderBallPath	= Enjon::String("Models/unit_shaderball.obj");
		Enjon::String catMeshPath			= Enjon::String("Models/cat.obj");
		Enjon::String dudeMeshPath			= Enjon::String("Models/dude.obj");
		Enjon::String shaderBallMeshPath	= Enjon::String("Models/shaderball.obj");
		Enjon::String cerebusAlbedoPath		= Enjon::String("Materials/Cerebus/Albedo.png"); 
		Enjon::String cerebusNormalPath		= Enjon::String("Materials/Cerebus/Normal.png"); 
		Enjon::String cerebusMetallicPath	= Enjon::String("Materials/Cerebus/Metallic.png"); 
		Enjon::String cerebusRoughnessPath	= Enjon::String("Materials/Cerebus/Roughness.png"); 
		Enjon::String cerebusEmissivePath	= Enjon::String("Materials/Cerebus/Emissive.png"); 
		Enjon::String mahogAlbedoPath		= Enjon::String("Materials/MahogFloor/Albedo.png"); 
		Enjon::String mahogNormalPath		= Enjon::String("Materials/MahogFloor/Normal.png"); 
		Enjon::String mahogMetallicPath		= Enjon::String("Materials/MahogFloor/Roughness.png"); 
		Enjon::String mahogRoughnessPath	= Enjon::String("Materials/MahogFloor/Roughness.png"); 
		Enjon::String mahogEmissivePath		= Enjon::String("Materials/MahogFloor/Emissive.png"); 
		Enjon::String mahogAOPath			= Enjon::String("Materials/MahogFloor/AO.png"); 
		Enjon::String woodAlbedoPath		= Enjon::String("Materials/WoodFrame/Albedo.png"); 
		Enjon::String woodNormalPath		= Enjon::String("Materials/WoodFrame/Normal.png"); 
		Enjon::String woodRoughnessPath		= Enjon::String("Materials/WoodFrame/Roughness.png"); 
		Enjon::String woodMetallicPath		= Enjon::String("Materials/WoodFrame/Metallic.png"); 
		Enjon::String plasticAlbedoPath		= Enjon::String("Materials/ScuffedPlastic/Albedo.png"); 
		Enjon::String plasticNormalPath		= Enjon::String("Materials/ScuffedPlastic/Normal.png"); 
		Enjon::String plasticRoughnessPath	= Enjon::String("Materials/ScuffedPlastic/Roughness.png"); 
		Enjon::String plasticMetallicPath	= Enjon::String("Materials/ScuffedPlastic/Metallic.png"); 
		Enjon::String plasticAOPath			= Enjon::String("Materials/ScuffedPlastic/AO.png"); 
		Enjon::String wornRedAlbedoPath		= Enjon::String("Materials/RustedIron/Albedo.png"); 
		Enjon::String wornRedNormalPath		= Enjon::String("Materials/RustedIron/Normal.png"); 
		Enjon::String wornRedRoughnessPath	= Enjon::String("Materials/RustedIron/Roughness.png"); 
		Enjon::String wornRedMetallicPath	= Enjon::String("Materials/RustedIron/Metallic.png"); 
		Enjon::String scuffedGoldAlbedoPath		= Enjon::String("Materials/ScuffedGold/Albedo.png"); 
		Enjon::String scuffedGoldNormalPath		= Enjon::String("Materials/ScuffedGold/Normal.png"); 
		Enjon::String scuffedGoldMetallicPath	= Enjon::String("Materials/ScuffedGold/Metallic.png"); 
		Enjon::String scuffedGoldRoughnessPath	= Enjon::String("Materials/ScuffedGold/Roughness.png"); 
		Enjon::String paintPeelingAlbedoPath = Enjon::String( "Materials/PaintPeeling/Albedo.png" );
		Enjon::String paintPeelingNormalPath = Enjon::String( "Materials/PaintPeeling/Normal.png" );
		Enjon::String paintPeelingRoughnessPath = Enjon::String( "Materials/PaintPeeling/Roughness.png" );
		Enjon::String paintPeelingMetallicPath = Enjon::String( "Materials/PaintPeeling/Metallic.png" );
		Enjon::String mixedMossAlbedoPath	= Enjon::String( "Materials/MixedMoss/Albedo.png" );
		Enjon::String mixedMossNormalPath	= Enjon::String( "Materials/MixedMoss/Normal.png" );
		Enjon::String mixedMossMetallicPath	= Enjon::String( "Materials/MixedMoss/Metallic.png" );
		Enjon::String mixedMossRoughnessPath	= Enjon::String( "Materials/MixedMoss/Roughness.png" );
		Enjon::String mixedMossAOPath		= Enjon::String( "Materials/MixedMoss/AO.png" );
		Enjon::String rockAlbedoPath		= Enjon::String("Materials/CopperRock/Albedo.png"); 
		Enjon::String rockNormalPath		= Enjon::String("Materials/CopperRock/Normal.png"); 
		Enjon::String rockRoughnessPath		= Enjon::String("Materials/CopperRock/Roughness.png"); 
		Enjon::String rockMetallicPath		= Enjon::String("Materials/CopperRock/Metallic.png"); 
		Enjon::String rockEmissivePath		= Enjon::String("Materials/CopperRock/Emissive.png"); 
		Enjon::String rockAOPath			= Enjon::String("Materials/CopperRock/AO.png"); 
		Enjon::String frontNormalPath		= Enjon::String("Textures/front_normal.png"); 
		Enjon::String brdfPath				= Enjon::String("Textures/brdf.png"); 
		Enjon::String waterPath				= Enjon::String("Textures/water.png"); 
		Enjon::String greenPath				= Enjon::String("Textures/green.png"); 
		Enjon::String redPath				= Enjon::String("Textures/red.png"); 
		Enjon::String bluePath				= Enjon::String("Textures/blue.png"); 
		Enjon::String blackPath				= Enjon::String("Textures/black.png"); 
		Enjon::String midGreyPath			= Enjon::String("Textures/grey.png"); 
		Enjon::String lightGreyPath			= Enjon::String("Textures/light_grey.png"); 
		Enjon::String whitePath				= Enjon::String("Textures/white.png"); 
		Enjon::String yellowPath			= Enjon::String("Textures/yellow.png"); 
		Enjon::String teapotPath			= Enjon::String( "Models/teapot.obj" );
		Enjon::String swordPath				= Enjon::String( "Models/sword.obj" );
		Enjon::String eyeNormal				= Enjon::String( "Textures/eye_NORMAL.png" );
		Enjon::String eyeAlbedo				= Enjon::String( "Textures/eyeball_COLOR1.png" );
		Enjon::String shaderGraphPath		= Enjon::String( "Shaders/ShaderGraphs/testGraph.sg" );
		Enjon::String staticGeomGraphPath	= Enjon::String( "Shaders/ShaderGraphs/DefaultStaticGeom.sg" ); 

		AssetManager* mAssetManager = EngineSubsystem( AssetManager );
		
		// Add to asset database( will serialize the asset if not loaded from disk, otherwise will load the asset )
		mAssetManager->AddToDatabase( toyBoxDispPath );
		mAssetManager->AddToDatabase( toyBoxNormalPath );
		mAssetManager->AddToDatabase( unitSpherePath );
		mAssetManager->AddToDatabase( shaderGraphPath );
		mAssetManager->AddToDatabase( staticGeomGraphPath );
		mAssetManager->AddToDatabase( eyePath );
		mAssetManager->AddToDatabase( eyeAlbedo );
		mAssetManager->AddToDatabase( eyeNormal );
		mAssetManager->AddToDatabase( dragonMeshPath );
		mAssetManager->AddToDatabase( swordPath );
		mAssetManager->AddToDatabase( noisePath );
		mAssetManager->AddToDatabase( grassTexturePath );
		mAssetManager->AddToDatabase( quadPath );
		mAssetManager->AddToDatabase( rockPath );
		mAssetManager->AddToDatabase( monkeyMeshPath );
		mAssetManager->AddToDatabase( cerebusAlbedoPath );
		mAssetManager->AddToDatabase( cerebusNormalPath );
		mAssetManager->AddToDatabase( cerebusMetallicPath );
		mAssetManager->AddToDatabase( cerebusRoughnessPath );
		mAssetManager->AddToDatabase( cerebusEmissivePath );
		mAssetManager->AddToDatabase( mahogAlbedoPath );
		mAssetManager->AddToDatabase( mahogNormalPath );
		mAssetManager->AddToDatabase( mahogMetallicPath );
		mAssetManager->AddToDatabase( mahogRoughnessPath );
		mAssetManager->AddToDatabase( mahogEmissivePath );
		mAssetManager->AddToDatabase( mahogAOPath );
		mAssetManager->AddToDatabase( woodAlbedoPath );
		mAssetManager->AddToDatabase( woodNormalPath );
		mAssetManager->AddToDatabase( woodMetallicPath );
		mAssetManager->AddToDatabase( woodRoughnessPath );
		mAssetManager->AddToDatabase( wornRedAlbedoPath );
		mAssetManager->AddToDatabase( wornRedRoughnessPath );
		mAssetManager->AddToDatabase( wornRedMetallicPath );
		mAssetManager->AddToDatabase( wornRedNormalPath );
		mAssetManager->AddToDatabase( plasticAlbedoPath );
		mAssetManager->AddToDatabase( plasticNormalPath );
		mAssetManager->AddToDatabase( plasticRoughnessPath );
		mAssetManager->AddToDatabase( plasticMetallicPath );
		mAssetManager->AddToDatabase( rockAlbedoPath );
		mAssetManager->AddToDatabase( rockNormalPath );
		mAssetManager->AddToDatabase( rockMetallicPath );
		mAssetManager->AddToDatabase( rockRoughnessPath );
		mAssetManager->AddToDatabase( rockEmissivePath );
		mAssetManager->AddToDatabase( rockAOPath );
		mAssetManager->AddToDatabase( scuffedGoldAlbedoPath );
		mAssetManager->AddToDatabase( scuffedGoldNormalPath );
		mAssetManager->AddToDatabase( scuffedGoldMetallicPath );
		mAssetManager->AddToDatabase( scuffedGoldRoughnessPath );
		mAssetManager->AddToDatabase( paintPeelingAlbedoPath );
		mAssetManager->AddToDatabase( paintPeelingNormalPath );
		mAssetManager->AddToDatabase( paintPeelingMetallicPath );
		mAssetManager->AddToDatabase( paintPeelingRoughnessPath );
		mAssetManager->AddToDatabase( mixedMossAlbedoPath );
		mAssetManager->AddToDatabase( mixedMossNormalPath );
		mAssetManager->AddToDatabase( mixedMossMetallicPath );
		mAssetManager->AddToDatabase( mixedMossRoughnessPath );
		mAssetManager->AddToDatabase( mixedMossAOPath );
		mAssetManager->AddToDatabase( plasticAOPath );
		mAssetManager->AddToDatabase( frontNormalPath );
		mAssetManager->AddToDatabase( brdfPath );
		mAssetManager->AddToDatabase( cerebusMeshPath );
		mAssetManager->AddToDatabase( sphereMeshPath );
		mAssetManager->AddToDatabase( cylinderMeshPath );
		mAssetManager->AddToDatabase( coneMeshPath );
		mAssetManager->AddToDatabase( cubeMeshPath );
		mAssetManager->AddToDatabase( catMeshPath );
		mAssetManager->AddToDatabase( bunnyMeshPath );
		mAssetManager->AddToDatabase( buddhaMeshPath );
		mAssetManager->AddToDatabase( dudeMeshPath );
		mAssetManager->AddToDatabase( shaderBallMeshPath );
		mAssetManager->AddToDatabase( unitShaderBallPath );
		mAssetManager->AddToDatabase( unrealShaderBallPath );
		mAssetManager->AddToDatabase( greenPath );
		mAssetManager->AddToDatabase( redPath );
		mAssetManager->AddToDatabase( midGreyPath );
		mAssetManager->AddToDatabase( lightGreyPath );
		mAssetManager->AddToDatabase( bluePath );
		mAssetManager->AddToDatabase( blackPath );
		mAssetManager->AddToDatabase( whitePath );
		mAssetManager->AddToDatabase( yellowPath );
		mAssetManager->AddToDatabase( teapotPath );
		mAssetManager->AddToDatabase( waterPath );

		// Create materials
		AssetHandle< Material > redMat = mAssetManager->ConstructAsset< Material >( "RedMaterial" );
		AssetHandle< Material > greenMat = mAssetManager->ConstructAsset< Material >( "GreenMaterial" );
		AssetHandle< Material > blueMat = mAssetManager->ConstructAsset< Material >( "BlueMaterial" );
		AssetHandle< Material > yellowMat = mAssetManager->ConstructAsset< Material >( "YellowMaterial" );
		AssetHandle< ShaderGraph > sg = mAssetManager->GetAsset< ShaderGraph >( "shaders.shadergraphs.defaultstaticgeom" );

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

		redMat->Save( );
		greenMat->Save( );
		blueMat->Save( );
		yellowMat->Save( ); 
	}
}


