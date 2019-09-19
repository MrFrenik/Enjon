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
#include <SDL2/SDL.h>

namespace fs = ghc::filesystem; 

typedef void( *funcSetEngineInstance )( Enjon::Engine* instance );
typedef Enjon::Application*( *funcCreateApp )( Enjon::Engine* );
typedef void( *funcDeleteApp )( Enjon::Application* ); 

void* dllHandleTemp = nullptr;
void* dllHandle = nullptr;
funcSetEngineInstance setEngineFunc = nullptr;
funcCreateApp createAppFunc = nullptr;
funcDeleteApp deleteAppFunc = nullptr;

using namespace Enjon;

String configuration = "Debug";

namespace Enjon
{ 
	//=========================================================================================

	SandboxApp::SandboxApp( const String& projectpath )
		: mProjectDir( projectpath )
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

			// Initialize the app
			app->Initialize( );

			// Turn on the physics simulation
			PhysicsSubsystem* physx = EngineSubsystem( PhysicsSubsystem );
			physx->PauseSystem( false ); 

			// Uh oh, how do...
			// Reinitialize asset manager
			AssetManager* am = EngineSubsystem( AssetManager );
			am->Reinitialize( mProjectDir + "Assets/" ); 
		}
	}

	//=========================================================================================

	Result SandboxApp::Initialize()
	{

	#ifdef ENJON_SYSTEM_WINDOWS
		dllHandle = SDL_LoadObject( "proj.dll" );
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
		else
		{ 
			std::cout << "Could not load library\n";
			return Result::FAILURE;
		} 

		return Result::SUCCESS;
	}

	//=========================================================================================

	Result SandboxApp::Update( f32 dt )
	{ 
		if ( mApp )
		{
			// Process application input
			mApp->ProcessInput( dt );

			// Update application ( ^ Could be called in the same tick )
			mApp->Update( dt ); 
		}

		return Result::PROCESS_RUNNING; 
	}

	//=========================================================================================

	Result SandboxApp::ProcessInput( f32 dt )
	{ 
		return Result::PROCESS_RUNNING; 
	}

	//=========================================================================================

	Result SandboxApp::Shutdown()
	{ 
		if (mApp)
		{
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

