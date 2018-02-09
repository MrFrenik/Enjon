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
Enjon::String mProjectsDir = "E:/Development/EnjonProjects/";
//Enjon::String mProjectsDir = "W:/Projects/";

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

	void EnjonEditor::InspectorView( bool* enabled )
	{
		if ( mSelectedEntity )
		{
			// Debug dump the entity ( Probably shouldn't do this and should tailor it more... )
			Entity* ent = mSelectedEntity.Get( ); 
			ImGuiManager::DebugDumpObject( ent );

			if ( ImGui::CollapsingHeader( "ADD COMPONENT" ) )
			{
				// Get component list
				EntityManager* entities = EngineSubsystem( EntityManager );
				auto compMetaClsList = entities->GetComponentMetaClassList( );

				ImGui::ListBoxHeader( "" );
				{
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
				}
				ImGui::ListBoxFooter( );
			}

			for ( auto& c : ent->GetComponents( ) )
			{
				if ( ImGui::CollapsingHeader( c->Class( )->GetName( ).c_str( ) ) )
				{
					ImGuiManager::DebugDumpObject( c );
				}
			}

			//if ( ImGui::TreeNode( fmt::format( "Components##{}", ent->GetID( ) ).c_str( ) ) )
			//{
			//	for ( auto& c : ent->GetComponents( ) )
			//	{ 
			//		if ( c->Class( ) == Object::GetClass<RigidBodyComponent >( ) )
			//		{
			//			RigidBodyComponent* rbc = c->ConstCast<RigidBodyComponent>( );
			//			if ( rbc )
			//			{
			//				f32 mass = rbc->GetMass( );
			//				if ( ImGui::DragFloat( "Mass", &mass, 1.0f, 0.0f, 100.0f ) )
			//				{
			//					rbc->SetMass( mass );
			//				}
			//			}
			//		}

			//		else
			//		{
			//			Enjon::ImGuiManager::DebugDumpObject( c ); 
			//		}
			//	} 
			//	ImGui::TreePop( );
			//}

		}
	}

	void EnjonEditor::SceneView( bool* viewBool )
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
		gfx->GetSceneCamera( )->ConstCast< Enjon::Camera >( )->SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );
	}

	Vec2 EnjonEditor::GetSceneViewProjectedCursorPosition( )
	{
		// Need to get percentage of screen and things and stuff from mouse position
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem ); 
		Input* input = EngineSubsystem( Input );
		iVec2 viewport = gfx->GetViewport( );
		Vec2 mp = input->GetMouseCoords( );
		
		// X screen percentage
		f32 pX = f32( mp.x - mSceneViewWindowPosition.x ) / f32( mSceneViewWindowSize.x );
		f32 pY = f32( mp.y - mSceneViewWindowPosition.y ) / f32( mSceneViewWindowSize.y ); 

		return Vec2( (s32)( pX * viewport.x ), (s32)( pY * viewport.y ) );
	}

	void EnjonEditor::CameraOptions( bool* enable )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
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

		if ( ImGui::TreeNode( "World Time" ) )
		{
			WorldTime wt = Engine::GetInstance( )->GetWorldTime( ); 
			if ( ImGui::SliderFloat( "Time Scale", &wt.mTimeScale, 0.001, 1.0f ) )
			{
				Engine::GetInstance( )->SetWorldTimeScale( wt.mTimeScale );
			}
			ImGui::TreePop( );
		}

		/*
		if ( ImGui::TreeNode( "XZAxis" ) )
		{
			ImGuiManager::DebugDumpObject( &mTransformWidget.mTranslationWidget.mXZAxis );

			ImGui::TreePop( );
		}
		if ( ImGui::TreeNode( "XYAxis" ) )
		{
			ImGuiManager::DebugDumpObject( &mTransformWidget.mTranslationWidget.mXYAxis );

			ImGui::TreePop( );
		}
		if ( ImGui::TreeNode( "YZAxis" ) )
		{
			ImGuiManager::DebugDumpObject( &mTransformWidget.mTranslationWidget.mYZAxis );

			ImGui::TreePop( );
		} 
		if ( ImGui::TreeNode( "TranslationRoot" ) )
		{
			ImGuiManager::DebugDumpObject( &mTransformWidget.mTranslationWidget.mRoot );

			ImGui::TreePop( );
		}
		*/
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

				GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
				auto cam = gfx->GetSceneCamera( )->ConstCast< Camera >();
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
				auto cam = gfx->GetSceneCamera( );
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
				LoadDLL( );
			}
		} 
	}

	void EnjonEditor::WorldOutlinerView( )
	{
		EntityManager* entities = EngineSubsystem( EntityManager );
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
						if ( c->Class( ) == Object::GetClass<RigidBodyComponent >( ) )
						{
							RigidBodyComponent* rbc = c->ConstCast<RigidBodyComponent>( );
							if ( rbc )
							{
								f32 mass = rbc->GetMass( );
								if ( ImGui::DragFloat( "Mass", &mass, 1.0f, 0.0f, 100.0f ) )
								{
									rbc->SetMass( mass );
								}
							}
						}

						else
						{
							Enjon::ImGuiManager::DebugDumpObject( c ); 
						}
					} 
					ImGui::TreePop( );
				}

				if ( ImGui::CollapsingHeader( "ADD COMPONENT" ) )
				{
					// Get component list
					EntityManager* entities = EngineSubsystem( EntityManager );
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
				Enjon::AssetManager* am = EngineSubsystem( AssetManager );
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

	void EnjonEditor::UnloadScene( )
	{
		EntityManager* em = EngineSubsystem( EntityManager );
		for ( auto& e : em->GetActiveEntities( ) )
		{
			e->Destroy( );
		}

		em->ForceCleanup( );
	}

	//================================================================================================================================

	void EnjonEditor::LoadProject( const Project& project )
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

	void EnjonEditor::LoadProjectSolution( )
	{
		// Now call BuildAndRun.bat
#ifdef ENJON_SYSTEM_WINDOWS 
		// TODO(): Error check the fuck out of this call
		// Is it possible to know whether or not this succeeded?
		s32 code = system( String( "start " + mProject.GetProjectPath() + "Build/" + mProject.GetProjectName() + ".sln" ).c_str() ); 
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
							// Load the project
							LoadProject( p );

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
		EntityManager* entities = EngineSubsystem( EntityManager );
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
		//EntityArchiver::Serialize( mSceneEntity, &buffer );

		// Destroy scene entity
		//mSceneEntity.Get()->Destroy( );

		// Clean mSceneEntities
		//mSceneEntities.clear( );

		// Force the scene to clean up ahead of frame
		//CleanupScene( ); 

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
			EntityManager* em = EngineSubsystem( EntityManager );
			mSceneEntities = em->GetActiveEntities( );

			// Initialize the app
			app->Initialize( );

			// Turn on the physics simulation
			PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
			physx->PauseSystem( false ); 
		}
	}

	//================================================================================================================================

	void EnjonEditor::ShutdownProjectApp( ByteBuffer* buffer )
	{
		Application* app = mProject.GetApplication( );
		if ( app )
		{
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

				// If either null or not in original cached entity list then destroy
				if ( !e || std::find( mSceneEntities.begin( ), mSceneEntities.end( ), e ) == mSceneEntities.end( ) )
				{
					e->Destroy( );
				}
			} 

			// Shutodwn the application
			app->Shutdown( ); 

			// Force the scene to clean up ahead of frame
			CleanupScene( ); 

			// Clean up physics subsystem from contact events as well
			PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
			phys->Reset( );

			// Pause the physics simulation
			phys->PauseSystem( true ); 
		}
	}
	 
	Enjon::Result EnjonEditor::Initialize( )
	{ 
		mApplicationName = "EnjonEditor"; 

		Enjon::String mAssetsDirectoryPath = Enjon::Engine::GetInstance()->GetConfig().GetRoot() + "Editor/Assets/";
		//Enjon::String cacheDirectoryPath = mAssetsDirectoryPath + "/Cache/";

		// Get asset manager and set its properties ( I don't like this )
		AssetManager* mAssetManager = EngineSubsystem( AssetManager );
		GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );

		// Pause the physics simulation
		physx->PauseSystem( true ); 

		// This also needs to be done through a config file or cmake
		//mAssetManager->SetAssetsDirectoryPath( mAssetsDirectoryPath );
		//mAssetManager->SetCachedAssetsDirectoryPath( cacheDirectoryPath );
		//mAssetManager->SetDatabaseName( GetApplicationName( ) );
		//mAssetManager->Initialize( ); 

		// Load all resources and cache them
		//LoadResources( );

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
		//EntityManager* entities = EngineSubsystem( EntityManager );
		//mSceneEntity = entities->Allocate( );
		//if ( mSceneEntity.Get() ) 
		//{ 
		//	Entity* ent = mSceneEntity.Get( );
		//	auto gfxComp = ent->AddComponent< GraphicsComponent >( );
		//	if ( gfxComp )
		//	{
		//		gfxComp->SetMesh( mAssetManager->GetAsset< Mesh >( "models.monkey" ) );
		//		gfxComp->SetMaterial( mAssetManager->GetDefaultAsset< Material >( ) );
		//	}
		//	ent->SetLocalPosition( Vec3( 5.0f, 2.0f, 4.0f ) );
		//} 

		//EntityHandle floorEnt = entities->Allocate( );
		//if ( floorEnt.Get() ) 
		//{ 
		//	Entity* ent = floorEnt.Get( );
		//	auto gfxComp = ent->AddComponent< GraphicsComponent >( );
		//	if ( gfxComp )
		//	{
		//		gfxComp->SetMesh( mAssetManager->GetAsset< Mesh >( "models.unit_cube" ) );
		//		gfxComp->SetMaterial( mAssetManager->GetAsset< Material >( "MahogFloorMaterial" ) );
		//	}
		//	ent->SetLocalScale( Vec3( 100.0f, 0.1f, 100.0f ) );
		//} 

		// Initialize transform widget
		mTransformWidget.Initialize( this );

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

		Enjon::ImGuiManager::RegisterWindow( [ & ]
		{
			if ( ImGui::BeginDock( "Inspector View", nullptr ) )
			{
				InspectorView( nullptr );
			}
			ImGui::EndDock( );
		} );

		mShowSceneView = true;
		auto sceneViewOption = [ & ] ( )
		{
			ImGui::MenuItem( "Scene##options", NULL, &mShowSceneView );
		};

		// Register menu options
		ImGuiManager::RegisterMenuOption( "View", sceneViewOption );

		// Register docking layouts 
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Scene", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Camera", "Scene", ImGui::DockSlotType::Slot_Right, 0.2f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Load Resource", "Camera", ImGui::DockSlotType::Slot_Bottom, 0.3f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "World Outliner", "Camera", ImGui::DockSlotType::Slot_Top, 0.7f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Play Options", "Scene", ImGui::DockSlotType::Slot_Top, 0.1f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Create Project", "Play Options", ImGui::DockSlotType::Slot_Tab, 0.1f ) );
		Enjon::ImGuiManager::RegisterDockingLayout( ImGui::DockingLayout( "Inspector View", "World Outliner", ImGui::DockSlotType::Slot_Bottom, 0.5f ) );

		return Enjon::Result::SUCCESS;
	}

	Enjon::Result EnjonEditor::Update( f32 dt )
	{
		static float t = 0.0f;
		t += dt;

		//GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		//const Camera* cam = gfx->GetSceneCamera( );

		//// Set scale of root based on distance from camera
		//f32 distSqred = Vec3::DistanceSquared( cam->GetPosition( ), mTransformWidget.GetWorldTransform( ).GetPosition( ) );
		//f32 scale = Clamp( distSqred, 0.5f, 100.0f );
		//mTransformWidget.SetScale( scale );

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

	Enjon::Result EnjonEditor::ProcessInput( f32 dt )
	{
		static bool mInteractingWithTransformWidget = false;
		static TransformMode mMode = TransformMode::Translate;
		static Vec3 mIntersectionStartPosition;
		static Vec3 mRootStartPosition;
		static Vec3 mRootStartScale;
		static TransformWidgetRenderableType mType;
		static
			GraphicsSubsystem* mGfx = EngineSubsystem( GraphicsSubsystem );
		Input* mInput = EngineSubsystem( Input );
		Camera* camera = mGfx->GetSceneCamera( )->ConstCast< Enjon::Camera >( );

		if ( mInput->IsKeyPressed( Enjon::KeyCode::T ) )
		{
			mMoveCamera ^= 1;
		}

		if ( mInput->IsKeyPressed( KeyCode::One ) )
		{
			mTransformWidget.SetTransformationMode( TransformationMode::Translation );
		}
		if ( mInput->IsKeyPressed( KeyCode::Two ) )
		{
			mTransformWidget.SetTransformationMode( TransformationMode::Scale );
		}

		if ( mInput->IsKeyDown( KeyCode::N ) )
		{
			Entity* ent = mSelectedEntity.Get( );
			if ( ent )
			{
				static f32 angle = 0.0f;
				angle += 10.0f * 0.001f; 
				Quaternion rotation = Quaternion::AngleAxis( ToRadians( angle ), Vec3::XAxis( ) );
				ent->SetLocalRotation( ent->GetLocalRotation( ) * rotation );
				mTransformWidget.SetRotation( ent->GetWorldRotation( ) );
			}
		} 

		if ( mInput->IsKeyDown( KeyCode::M ) )
		{
			Entity* ent = mSelectedEntity.Get( );
			if ( ent )
			{
				static f32 angle = 0.0f;
				angle += 10.0f * 0.001f;
				Quaternion rotation = Quaternion::AngleAxis( ToRadians( angle ), Vec3::YAxis( ) );
				ent->SetLocalRotation( ent->GetLocalRotation( ) * rotation );
				mTransformWidget.SetRotation( ent->GetWorldRotation( ) );
			}
		} 

		if ( mInput->IsKeyDown( KeyCode::Comma ) )
		{
			Entity* ent = mSelectedEntity.Get( );
			if ( ent )
			{
				static f32 angle = 0.0f;
				angle += 10.0f * 0.001f;;
				Quaternion rotation = Quaternion::AngleAxis( ToRadians( angle ), Vec3::ZAxis( ) );
				ent->SetLocalRotation( ent->GetLocalRotation( ) * rotation );
				mTransformWidget.SetRotation( ent->GetWorldRotation( ) );
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
			//Enjon::Vec2 mouseCoords = mInput->GetMouseCoords( );

			Vec2 mouseCoords = mInput->GetMouseCoords( );

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
					mTransformWidget.SetRotation( pr.mEntity.Get( )->GetWorldRotation( ) );
					mTransformWidget.SetPosition( pr.mEntity.Get( )->GetWorldPosition( ) );
					mSelectedEntity = pr.mEntity;
				}
				// Translation widget interaction
				else
				{
					// Begin widget interaction
					mTransformWidget.BeginWidgetInteraction( TransformWidgetRenderableType( pr.mId - MAX_ENTITIES ) );
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

		return Enjon::Result::PROCESS_RUNNING;
	}

	Enjon::Result EnjonEditor::Shutdown( )
	{ 
		return Enjon::Result::SUCCESS;
	} 

	void EnjonEditor::LoadResources( )
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


