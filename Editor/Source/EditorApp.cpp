#include "EditorApp.h"

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
Enjon::String mProjectsDir = "E:/Development/EnjonProjects/";
//Enjon::String mProjectsDir = "W:/Projects/";

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

	void SceneView( bool* viewBool )
	{
		const Enjon::GraphicsSubsystem* gfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( );
		u32 currentTextureId = gfx->GetCurrentRenderTextureId( ); 

		// Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

		ImTextureID img = ( ImTextureID )currentTextureId;
		ImGui::Image( img, ImVec2( ImGui::GetWindowWidth( ), ImGui::GetWindowHeight( ) ),
			ImVec2( 0, 1 ), ImVec2( 1, 0 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 0 ) );

		ImVec2 min = ImVec2( cursorPos.x + ImGui::GetContentRegionAvailWidth( ) - 100.0f, cursorPos.y + 10.0f );
		ImVec2 max = ImVec2( min.x + 50.0f, min.y + 10.0f );

		ImGui::SetCursorScreenPos( min );
		auto drawlist = ImGui::GetWindowDrawList( );
		f32 fps = ImGui::GetIO( ).Framerate;

		// Update camera aspect ratio
		gfx->GetSceneCamera( )->ConstCast< Enjon::Camera >( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );
	}

	void EnjonEditor::CameraOptions( bool* enable )
	{
		const Enjon::GraphicsSubsystem* gfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( ); 
		const Enjon::Camera* cam = gfx->GetSceneCamera( );

		if ( ImGui::TreeNode( "Camera" ) )
		{
			Enjon::ImGuiManager::DebugDumpObject( cam ); 
			ImGui::TreePop( );
		}

		ImGui::DragFloat( "Camera Speed", &mCameraSpeed, 0.1f ); 

		if ( ImGui::TreeNode( "Application" ) )
		{
			if ( mProject.GetApplication() )
			{
				Enjon::ImGuiManager::DebugDumpObject( mProject.GetApplication() ); 
			}
			ImGui::TreePop( );
		} 
	}

	void EnjonEditor::PlayOptions( )
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

				auto cam = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetSceneCamera( )->ConstCast< Enjon::Camera >();
				cam->SetPosition( mPreviousCameraTransform.Position );
				cam->SetRotation( mPreviousCameraTransform.Rotation ); 
			}
		}
		else
		{
			if ( ImGui::Button( "Play" ) )
			{ 
				mPlaying = true;
				mMoveCamera = true;

				auto cam = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetSceneCamera( ); 
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
	}

	void EnjonEditor::WorldOutlinerView( )
	{
		const Enjon::EntityManager* entities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( );
		ImGui::Text( fmt::format( "Entities: {}", entities->GetActiveEntities().size() ).c_str() ); 

		for ( auto& e : entities->GetActiveEntities( ) )
		{
			if ( ImGui::TreeNode( fmt::format( "{}", e->GetID( ) ).c_str( ) ) )
			{
				Enjon::ImGuiManager::DebugDumpObject( e );

				if ( ImGui::TreeNode( fmt::format( "Components##{}", e->GetID( ) ).c_str( ) ) )
				{
					for ( auto& c : e->GetComponents( ) )
					{
						Enjon::ImGuiManager::DebugDumpObject( c );
					} 
					ImGui::TreePop( );
				}

				if ( ImGui::CollapsingHeader( "ADD COMPONENT" ) )
				{
					// Get component list
					auto entities = Engine::GetInstance( )->GetSubsystem( Object::GetClass< EntityManager >( ) )->ConstCast< EntityManager >( );
					auto compMetaClsList = entities->GetComponentMetaClassList( );

					ImGui::ListBoxHeader( "" );
					{
						for ( auto& cls : compMetaClsList )
						{
							if ( !mSceneEntity.Get( )->HasComponent( cls ) )
							{
								// Add component to mEntity
								if ( ImGui::Selectable( cls->GetName( ).c_str( ) ) )
								{
									mSceneEntity.Get( )->AddComponent( cls );
								} 
							}
						} 
					}
					ImGui::ListBoxFooter( );
				}

				ImGui::TreePop( );
			}
		}
	}

	void EnjonEditor::LoadResourceFromFile( )
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
				Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( )->ConstCast< Enjon::AssetManager >( );
				am->AddToDatabase( mResourceFilePathToLoad );
			}
		}
	}

	//================================================================================================================================
	 
	void EnjonEditor::CreateNewProject( const String& projectName )
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
		s32 code = system( String( projectDir + "Proc/" + "BuildAndRun.bat" + " " + Enjon::Utils::FindReplaceAll( projectDir, "/", "\\" ) + " " + projectName ).c_str() ); 
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

	void EnjonEditor::LoadProjectSolution( )
	{
		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Error check the fuck out of this call
		// Is it possible to know whether or not this succeeded?
		s32 code = system( String( "call " + mProject.GetProjectPath() + "Build/" + mProject.GetProjectName() + ".sln" ).c_str() ); 
#endif
	}

	//================================================================================================================================

	void EnjonEditor::CreateProjectView( )
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
			if ( ImGui::CollapsingHeader( "Existing Projects" ) )
			{
				ImGui::ListBoxHeader( "##projects" );
				{
					for ( auto& p : mProjectsOnDisk )
					{
						if ( ImGui::Selectable( p.GetProjectName( ).c_str( ) ) )
						{ 
							// Unload previous dll
							UnloadDLL( );

							// Set project
							mProject = p;

							// Load project dll
							LoadDLL( );
						}
					}
				}
				ImGui::ListBoxFooter( );
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

	void EnjonEditor::CleanupScene( )
	{ 
		// Force the scene to clean up ahead of frame
		auto entities = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< EntityManager >( )->ConstCast< EntityManager >( );
		entities->ForceCleanup( );
	}

	//================================================================================================================================

	bool EnjonEditor::UnloadDLL( ByteBuffer* buffer )
	{
		bool needsReload = false;

		if ( dllHandle )
		{
			if ( mPlaying && buffer )
			{
				Application* app = mProject.GetApplication( );
				if ( app )
				{
					// NOTE(): Will not work if the layout of the data has changed! Need versioning working first...
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

	void EnjonEditor::LoadDLL( )
	{
		Enjon::ByteBuffer buffer;

		// NOTE(): VERY SPECIFIC IMPL THAT WILL BE GENERALIZED TO RELOADING SCENE
		// Serialize mEntity ( this will be the scene, essentially... )
		EntityArchiver::Serialize( mSceneEntity, &buffer );

		// Destroy scene entity
		mSceneEntity.Get()->Destroy( );

		// Clean mSceneEntities
		mSceneEntities.clear( );

		// Force the scene to clean up ahead of frame
		CleanupScene( ); 

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
	}

	//================================================================================================================================

	void EnjonEditor::CollectAllProjectsOnDisk( )
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

	void EnjonEditor::InitializeProjectApp( )
	{
		Application* app = mProject.GetApplication( );
		if ( app )
		{
			// Cache off all entity handles in scene before app starts
			mSceneEntities = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->GetActiveEntities( );

			// Initialize the app
			app->Initialize( );
		}
	}

	//================================================================================================================================

	void EnjonEditor::ShutdownProjectApp( ByteBuffer* buffer )
	{
		Application* app = mProject.GetApplication( );
		if ( app )
		{
			// Destroy any entities alive that aren't in the cached off entity list
			for ( auto& e : Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::EntityManager >( )->GetActiveEntities( ) )
			{ 
				// If either null or not in original cached entity list then destroy
				if ( !e || std::find( mSceneEntities.begin( ), mSceneEntities.end( ), e ) == mSceneEntities.end( ) )
				{
					e->Destroy( );
				}
			} 

			// Shutodwn the application
			app->Shutdown( );

			// Shutdown all entity components ( should put this lower in the engine code...)
			for ( auto& e : Engine::GetInstance( )->GetSubsystemCatalog( )->Get< EntityManager >( )->GetActiveEntities( ) )
			{
				for ( auto& c : e->GetComponents( ) )
				{
					c->Shutdown( );
				}
			} 

			// Force the scene to clean up ahead of frame
			CleanupScene( ); 

			// Clean up physics subsystem from contact events as well
			auto phys = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::PhysicsSubsystem >( )->ConstCast< Enjon::PhysicsSubsystem >( );
			phys->Reset( );
		}
	}
	 
	Enjon::Result EnjonEditor::Initialize( )
	{ 
		mApplicationName = "EnjonEditor";

		Enjon::String mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + "Editor/Assets/";
		Enjon::String cacheDirectoryPath = mAssetsDirectoryPath + "/Cache/";

		// Get asset manager and set its properties ( I don't like this )
		Enjon::AssetManager* mAssetManager = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get<Enjon::AssetManager>( )->ConstCast< Enjon::AssetManager >( );
		Enjon::GraphicsSubsystem* mGfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( );

		// This also needs to be done through a config file or cmake
		mAssetManager->SetAssetsDirectoryPath( mAssetsDirectoryPath );
		mAssetManager->SetCachedAssetsDirectoryPath( cacheDirectoryPath );
		mAssetManager->SetDatabaseName( GetApplicationName( ) );
		mAssetManager->Initialize( ); 

		// Register project template files
		mProjectSourceTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectSourceTemplate.cpp" ).c_str() ); 
		mProjectCMakeTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/ProjectCMakeTemplate.txt" ).c_str( ) );
		mProjectDelBatTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/DelPDB.bat" ).c_str( ) );
		mProjectBuildAndRunTemplate = Enjon::Utils::read_file_sstream( ( mAssetsDirectoryPath + "ProjectTemplates/BuildAndRun.bat" ).c_str( ) ); 

		// Set up copy directory for project dll
		copyDir = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( ) + projectName + "/";

		// Grab all .eproj files and store them for loading later
		CollectAllProjectsOnDisk( );

		// Initialize scene entity
		EntityManager* entities = Engine::GetInstance( )->GetSubsystem( Object::GetClass<EntityManager >( ) )->ConstCast< EntityManager >( );
		mSceneEntity = entities->Allocate( );
		if ( mSceneEntity.Get() ) 
		{ 
			Entity* ent = mSceneEntity.Get( );
			auto gfxComp = ent->AddComponent< GraphicsComponent >( );
			if ( gfxComp )
			{
				gfxComp->SetMesh( mAssetManager->GetAsset< Mesh >( "models.monkey" ) );
				gfxComp->SetMaterial( mAssetManager->GetDefaultAsset< Material >( ) );
			}
			ent->SetLocalPosition( Vec3( 5.0f, 2.0f, 4.0f ) );
		}

		// Register individual windows
		Enjon::ImGuiManager::RegisterWindow( [ & ] ( )
		{
			// Docking windows
			if ( ImGui::BeginDock( "Scene", &mShowSceneView, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
			{
				// Print docking information
				SceneView( &mShowSceneView );
			}
			ImGui::EndDock( ); 
		}); 

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

		mShowSceneView = true;
		auto sceneViewOption = [&]()
		{
        	ImGui::MenuItem("Scene##options", NULL, &mShowSceneView);
		};

		// Register menu options
		ImGuiManager::RegisterMenuOption("View", sceneViewOption);

		// Register docking layouts 
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Camera", "Scene", ImGui::DockSlotType::Slot_Right, 0.2f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Load Resource", "Camera", ImGui::DockSlotType::Slot_Bottom, 0.3f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "World Outliner", "Camera", ImGui::DockSlotType::Slot_Top, 0.7f ) ); 
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Play Options", "Scene", ImGui::DockSlotType::Slot_Top, 0.1f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Create Project", "Play Options", ImGui::DockSlotType::Slot_Tab, 0.1f ) );

		return Enjon::Result::SUCCESS;
	}

	Enjon::Result EnjonEditor::Update( f32 dt )
	{ 
		static float t = 0.0f;
		t += dt; 

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

	Enjon::Result EnjonEditor::ProcessInput( f32 dt )
	{ 
		const Enjon::GraphicsSubsystem* mGfx = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem>( );
		const Enjon::Input* mInput = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::Input >( );
		Enjon::Camera* camera = mGfx->GetSceneCamera( )->ConstCast< Enjon::Camera >(); 

		if ( mInput->IsKeyPressed( Enjon::KeyCode::T ) )
		{
			mMoveCamera ^= 1;
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

			// Set camera position
			camera->Transform.Position += dt * mCameraSpeed * velDir;

			// Set camera rotation
			// Get mouse input and change orientation of camera
			Enjon::Vec2 mouseCoords = mInput->GetMouseCoords( );

			Enjon::iVec2 viewPort = mGfx->GetViewport( );

			f32 mouseSensitivity = 10.0f;

			// Grab window from graphics subsystem
			Enjon::Window* window = mGfx->GetWindow( )->ConstCast< Enjon::Window >( );

			// Set cursor to not visible
			window->ShowMouseCursor( false );

			// Reset the mouse coords after having gotten the mouse coordinates
			SDL_WarpMouseInWindow( window->GetWindowContext( ), ( f32 )viewPort.x / 2.0f, ( f32 )viewPort.y / 2.0f );

			// Offset camera orientation
			f32 xOffset = Enjon::ToRadians( ( f32 )viewPort.x / 2.0f - mouseCoords.x ) * dt * mouseSensitivity;
			f32 yOffset = Enjon::ToRadians( ( f32 )viewPort.y / 2.0f - mouseCoords.y ) * dt * mouseSensitivity;
			camera->OffsetOrientation( xOffset, yOffset );
		}
		else
		{
			mGfx->GetWindow( )->ConstCast< Enjon::Window >( )->ShowMouseCursor( true );
		}

		// DLL Loading/Unloading
		{
			if ( mInput->IsKeyPressed( Enjon::KeyCode::R ) )
			{
				LoadDLL( );
			}
		}

		// Starting /Stopping game instance
		{
			if ( mInput->IsKeyPressed( Enjon::KeyCode::P ) )
			{
				if ( !mPlaying )
				{
					mPlaying = true;
					mMoveCamera = true;

					auto cam = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetSceneCamera( );
					mPreviousCameraTransform = Enjon::Transform( cam->GetPosition(), cam->GetRotation(), Enjon::Vec3( cam->GetOrthographicScale() ) );

					// Call start up function for game
					Application* app = mProject.GetApplication( );
					if ( app )
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

					auto cam = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::GraphicsSubsystem >( )->ConstCast< Enjon::GraphicsSubsystem >( )->GetSceneCamera( )->ConstCast< Enjon::Camera >( );
					cam->SetPosition( mPreviousCameraTransform.Position );
					cam->SetRotation( mPreviousCameraTransform.Rotation );
				}
			}
		}

		return Enjon::Result::PROCESS_RUNNING;
	}

	Enjon::Result EnjonEditor::Shutdown( )
	{ 
		return Enjon::Result::SUCCESS;
	} 
}


