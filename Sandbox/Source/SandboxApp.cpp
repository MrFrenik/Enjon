#include "SandboxApp.h"

#include <Engine.h>
#include <System/Config.h> 
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

#include <fs/filesystem.hpp> 
#include <zmq/zmq.hpp>
#include <SDL2/SDL.h>

#include <thread>

namespace fs = ghc::filesystem; 

typedef void( *funcSetEngineInstance )( Enjon::Engine* instance );
typedef Enjon::Application*( *funcCreateApp )( Enjon::Engine* );
typedef void( *funcDeleteApp )( Enjon::Application* ); 

void* dllHandleTemp = nullptr;
void* dllHandle = nullptr;
funcSetEngineInstance setEngineFunc = nullptr;
funcCreateApp createAppFunc = nullptr;
funcDeleteApp deleteAppFunc = nullptr;

std::thread* zmqThread = nullptr;

using namespace Enjon;

String configuration = "Debug";

namespace Enjon
{ 
	//=========================================================================================

	// This has to happen anyway. Perfect!
#ifdef ENJON_SYSTEM_WINDOWS
	void _CopyLibraryContentsInternal( const String& projectName, const String& projectDir, const String& config, const String& projDllName )
	{
		String rootDir = Engine::GetInstance()->GetConfig().GetRoot();
		String dllName = projectName + ".dll";
		{
			String dllPath = rootDir + "Build/" + config + "/" + projDllName + ".dll";
			bool exists = fs::exists( dllPath );
			if ( exists )
			{
				fs::remove( dllPath );
			}
			dllPath = projectDir;
			if ( fs::exists( dllPath ) )
			{
				if ( fs::exists( dllPath + "Build/" + config + "/" + dllName ) )
				{
					fs::copy( fs::path( dllPath + "Build/" + config + "/" + dllName ), rootDir + "Build/" + config + "/" + projDllName + ".dll" );
				}
			}
		}
	}

	void CopyLibraryContents( const String& projectName, const String& projectDir )
	{ 
		_CopyLibraryContentsInternal( projectName, projectDir, "Debug", "proj_s" );
		_CopyLibraryContentsInternal( projectName, projectDir, "Release", "proj_s" );
	}
#endif 

	SandboxApp::SandboxApp( const String& projectpath, const String& projectName )
		: mProjectDir( projectpath ), mProjectName( projectName )
	{ 
	}

	//=========================================================================================

	void SandboxApp::InitializeProjectApp()
	{
		// Get project application
		Application* app = mApp; 

		// If both are loaded then can initialize application
		if ( app )
		{
			// Set application state to running
			SetApplicationState( ApplicationState::Running );

			// Cache off all entity handles in scene before app starts
			EntityManager* em = EngineSubsystem( EntityManager ); 

			// Reinitialize asset manager based on asset path for project
			AssetManager* am = EngineSubsystem( AssetManager );
			am->Reinitialize( mProjectDir + "Assets/" ); 

			// Turn on the physics simulation
			PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
			physx->PauseSystem( true ); 

			GUIContext* gCtx = EngineSubsystem( WindowSubsystem )->GetWindows( ).at( 0 )->GetGUIContext( );
			gCtx->ClearContext( );
			GUIContextParams params;
			params.mUseRootDock = false;
			gCtx->SetGUIContextParams( params );
			gCtx->Finalize( );

			// Initialize the app
			app->Initialize( ); 
		}
	}

	//=========================================================================================

	void SandboxApp::UnloadDLL()
	{
		if ( dllHandle )
		{
			//if ( mPlaying && buffer )
			//{
			//	Application* app = mProject.GetApplication( );
			//	if ( app )
			//	{
			//		Enjon::ObjectArchiver::Serialize( app, buffer );
			//		needsReload = true;

			//		// Shutdown project app
			//		ShutdownProjectApp( buffer );
			//	}
			//}

			// Shutdown the app
			mApp->Shutdown();

			// Release scene
			EngineSubsystem( SceneManager )->UnloadScene();

			// Destroy entities
			EngineSubsystem( EntityManager )->DestroyAll();
			EngineSubsystem( EntityManager )->ForceCleanup();

			// Clear gui context
			EngineSubsystem( WindowSubsystem )->GetWindows().at( 0 )->GetGUIContext()->ClearContext(); 

			// Free application memory
			if ( deleteAppFunc )
			{
				deleteAppFunc( mApp );
				mApp = nullptr;
			}

			// Free library if in use
			// FreeLibrary( dllHandle );
			SDL_UnloadObject( dllHandle );
			dllHandle = nullptr;
			createAppFunc = nullptr;
			deleteAppFunc = nullptr;
		}

	}

	void SandboxApp::LoadDLL()
	{
	#ifdef ENJON_SYSTEM_WINDOWS
		dllHandle = SDL_LoadObject( "proj_s.dll" );
	#endif

		assert( dllHandle );

		// If valid, then set address of procedures to be called
		if ( dllHandle )
		{
			// Set functions from handle
			createAppFunc = (funcCreateApp)SDL_LoadFunction( dllHandle, "CreateApplication" );
			deleteAppFunc = (funcDeleteApp)SDL_LoadFunction( dllHandle, "DeleteApplication" );

			// Create application
			if ( createAppFunc )
			{
				mApp = createAppFunc( Enjon::Engine::GetInstance() );
			}

			// Call start up function for game
			if ( mApp )
			{
				InitializeProjectApp( );
			}
		} 
	}

	void SandboxApp::ReloadDLL()
	{
		// Save scene temporarily

		// Unload the library
		UnloadDLL(); 

		// Copy over library contents
		CopyLibraryContents( mProjectName, mProjectDir ); 
		
		// Load library
		LoadDLL();

		// Reload scene in place
	}

	Result SandboxApp::Initialize()
	{ 
		LoadDLL(); 
		SetupLocalClient();

		return Result::SUCCESS;
	}

	//=========================================================================================

	void SandboxApp::SetupLocalClient()
	{ 
		zmqThread = new std::thread( [&]() 
		{ 
			zmq::context_t context = zmq::context_t( 1 );
			zmq::socket_t socket = zmq::socket_t( context, ZMQ_REQ );
			socket.connect( "tcp://localhost:5555" ); 

			while ( true ) 
			{
				for ( u32 req_n = 0; req_n != 10; ++req_n )
				{
					//zmq::message_t req( 4 );
					//memcpy( req.data(), "Quit", 4 );
					//socket.send( req ); 
					//zmq::message_t rep;
					//if ( socket.recv( &rep ) ) {
					//	std::cout << rep.str() << "\n";
					//}
				} 
			}
		});
	}

	Result SandboxApp::Update( f32 dt )
	{ 
		Result res = Result::PROCESS_RUNNING;

		if ( mApp )
		{
			// Process application input
			res = mApp->ProcessInput( dt );
			if ( res != Result::PROCESS_RUNNING ) {
				return res;
			}

			// Update application ( ^ Could be called in the same tick )
			res = mApp->Update( dt ); 
			if ( res != Result::PROCESS_RUNNING ) {
				return res;
			}
		}

		if ( mNeedsDLLReload )
		{
			ReloadDLL();
			mNeedsDLLReload = false;
		}

		return Result::PROCESS_RUNNING; 
	}

	//=========================================================================================

	Result SandboxApp::ProcessInput( f32 dt )
	{ 
		Input* input = EngineSubsystem( Input );
		if ( input->IsKeyPressed( KeyCode::R ) && input->IsKeyDown( KeyCode::LeftCtrl ) )
		{
			mNeedsDLLReload = true;
		}

		return Result::PROCESS_RUNNING; 
	}

	//=========================================================================================

	Result SandboxApp::Shutdown()
	{ 
		if (mApp)
		{
			zmq::context_t context = zmq::context_t( 1 );
			zmq::socket_t socket = zmq::socket_t( context, ZMQ_REQ );
			socket.connect( "tcp://localhost:5555" ); 
			zmq::message_t req( 4 );
			memcpy( req.data(), "Quit", 4 );
			socket.send( req ); 

			// Shutdown application state
			SetApplicationState( ApplicationState::Stopped ); 

			SceneManager* sm = EngineSubsystem( SceneManager );

			// Shutodwn the application
			mApp->Shutdown( ); 

			// Unload current scene and catch its uuid
			UUID uuid = sm->UnloadScene( ); 

			// Clean up physics subsystem from contact events as well
			PhysicsSubsystem* phys = EngineSubsystem( PhysicsSubsystem );
			phys->Reset( );

			// Pause the physics simulation
			phys->PauseSystem( true ); 
		}

		return Result::SUCCESS; 
	}

	//=========================================================================================
}

