// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// File: Engine.cpp

#include "Engine.h"
#include "Application.h"
#include "Graphics/GraphicsSubsystem.h"
#include "Entity/EntityManager.h"
#include "Base/MetaClassRegistry.h"
#include "Base/Object.h"
#include "Asset/AssetManager.h"
#include "IO/InputManager.h"
#include "ImGui/ImGuiManager.h"
#include "Physics/PhysicsSubsystem.h"
#include "Graphics/AnimationSubsystem.h"
#include "Scene/SceneManager.h"
#include "Utils/Timing.h"
#include "SubsystemCatalog.h"

#include <fmt/printf.h> 
#include <SDL2/sdl.h>

#include <assert.h>
#include <random>
#include <time.h>

// Totally temporary
static bool mMovementOn = false;

 Enjon::Utils::FPSLimiter mLimiter;

namespace Enjon
{
	Engine* Engine::mInstance = nullptr; 

	//=======================================================

	Engine::Engine()
	{
		assert(mInstance == nullptr);
		mInstance = this;
	}	

	//=======================================================

	Engine::~Engine()
	{ 
		// Shutdown imguimanager
		//ImGuiManager::ShutDown( );

		// Shutdown all subsystems
		delete( mSubsystemCatalog );
		mSubsystemCatalog = nullptr; 

		// Shutdown meta class registry
		delete( mMetaClassRegisty );
		mMetaClassRegisty = nullptr;
	}

	//=======================================================

	Enjon::Result Engine::StartUp(const EngineConfig& config)
	{
		// TODO(): Find out where this should be abstracted into
		 //Initialize SDL
		SDL_Init(SDL_INIT_EVERYTHING);
		
		//Tell SDL that we want a double buffered window so we don't get any flickering
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,      8);
		 
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,        32);
		 
		SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);

		// Set on vsync by default
		SDL_GL_SetSwapInterval( 1 );

		// Set configuration
		mConfig = config;

		InitSubsystems();

		return Enjon::Result::SUCCESS;
	}

	//=======================================================

	Enjon::Result Engine::StartUp(Application* app, const EngineConfig& config)
	{
		// Register application 
		Enjon::Result res = RegisterApplication(app);

		if (res != Enjon::Result::SUCCESS)
		{
			// Error here
			assert(false);
			return res;
		}

		// Start up engine and subsystems
		res = StartUp(config);
		if (res != Enjon::Result::SUCCESS)
		{
			// Error here
			assert(false);
			return res;
		}

		// Return result
		return res;
	}

	//=======================================================

	Engine* Engine::GetInstance()
	{
		return mInstance;
	}
	
	//=======================================================
			
	const MetaClassRegistry* Engine::GetMetaClassRegistry( )
	{
		return mMetaClassRegisty;
	}
	
	//=======================================================
			
	const EngineConfig& Engine::GetConfig() const
	{
		return mConfig;
	}
	
	//=======================================================

	void Engine::SetIsStandAloneApplication( bool enabled )
	{
		mConfig.SetIsStandAloneApplication( enabled );
	}
	
	//=======================================================

	Enjon::Result Engine::InitSubsystems()
	{ 
		// Create new subsystem catalog
		mSubsystemCatalog = new SubsystemCatalog();

		// Meta class registration
		mMetaClassRegisty = new MetaClassRegistry( );

		// Register all base object meta classes
		Enjon::Object::BindMetaClasses( );

		// Register and bind all application specific meta classes
		mApp->BindApplicationMetaClasses( ); 

		// Register imgui manager and initialize
		mImGuiManager	= mSubsystemCatalog->Register< ImGuiManager >( false ); 

		// Register remaining subsystems
		mAssetManager		= mSubsystemCatalog->Register< AssetManager >( false );		// Will do manual initialization of asset management system, since it's project dependent
		mGraphics			= mSubsystemCatalog->Register< GraphicsSubsystem >( );
		mInput				= mSubsystemCatalog->Register< Input >( ); 
		mEntities			= mSubsystemCatalog->Register< EntityManager >( );
		mPhysics			= mSubsystemCatalog->Register< PhysicsSubsystem >( );
		mSceneManager		= mSubsystemCatalog->Register< SceneManager >( );
		mAnimationSystem	= mSubsystemCatalog->Register< AnimationSubsystem >( );

		// Default setting for assets directory
		mAssetManager->SetAssetsDirectoryPath( mConfig.GetRoot( ) + "Assets/" );
		mAssetManager->Initialize( ); 

		// Initialize imgui manager
		mImGuiManager->Init( mGraphics->GetWindow( )->ConstCast< Window >( )->GetSDLWindow( ) );

		// Initialize application if one is registered
		if ( mApp )
		{
			mApp->Initialize();
		} 

		// Initializes limiter
		 mLimiter.Init( 60.0f );

		// Late init for systems that need it
		 mImGuiManager->LateInit( mGraphics->GetWindow( )->ConstCast< Window >( )->GetSDLWindow( ) );

		return Enjon::Result::SUCCESS;
	}

	//=======================================================

	const Subsystem* Engine::GetSubsystem( const MetaClass* cls ) const
	{
		if ( cls && mInstance )
		{
			return mInstance->GetSubsystemCatalog( )->Get( cls );
		}

		return nullptr;
	}

	//=======================================================

	Enjon::Result Engine::RegisterApplication( Application* app )
	{
		assert( mApp == nullptr );
		mApp = app;

		return Enjon::Result::SUCCESS;
	}
	
	//=======================================================

	Enjon::Result Engine::Run()
	{
		//static float dt = 0.01f;

		// Assert that application is registered with engine
		assert( mApp != nullptr ); 

		// Seed random 
		srand( time( NULL ) ); 

		// Main application loop
		bool mIsRunning = true;
		while (mIsRunning)
		{
			static u32 thisTime = 0;
			static u32 lastTime = 0;
			f32 dt = 0.0f;
 
			thisTime = SDL_GetTicks( );
			u32 ticks = thisTime - lastTime;
			dt = ( f32 )( ticks ) / 1000.0f;
			lastTime = thisTime; 

			 // Update input manager
			mInput->Update( dt ); 

			// Update physics
			mPhysics->Update( dt ); 

			// Update input
			Enjon::Result res = ProcessInput( mInput, dt );
			if ( res != Result::PROCESS_RUNNING )
			{
				mIsRunning = false;
				break;
			} 
			
			// Process application input
			res = mApp->ProcessInput( dt );
			if ( res != Result::PROCESS_RUNNING )
			{
				mIsRunning = false;
				break;
			}

			// Update application 
			res = mApp->Update( dt );
			if ( res != Result::PROCESS_RUNNING )
			{
				mIsRunning = false;
				break;
			}

			// Update entity manager
			mEntities->Update( dt ); 

			// Update graphics
			mGraphics->Update( dt ); 

			// Update world time
			mWorldTime.mDT = dt;
			mWorldTime.mTotalTime += mWorldTime.mDT;
			mWorldTime.mFPS = 1000.0f / Math::Max( mWorldTime.mDT, 0.00001f ); 

			// Calculate average delta time for world time
			mWorldTime.CalculateAverageDeltaTime( );
			
			// Clamp frame rate to ease up on CPU usage
			static f32 t = 0.0f;
			const f32 frameRate = 120.0f;
			if ( (f32)ticks < 1000.0f / frameRate )
			{
				SDL_Delay( u32( (1000.0f / frameRate ) - ( (f32)ticks ) ) );
			}
		}

		Enjon::Result res = ShutDown();

		return res;
	}

	//======================================================= 

	Enjon::Result Engine::ShutDown()
	{
		if ( mApp )
		{
			mApp->Shutdown();
		}

		return Enjon::Result::SUCCESS;
	}

	//======================================================= 

	WorldTime Engine::GetWorldTime( ) const
	{
		return mWorldTime;
	}

	//======================================================= 

	const Application* Engine::GetApplication( )
	{
		return mApp;
	}

	//======================================================= 

	GraphicsSubsystem* Engine::GetGraphicsSubsystem( ) const
	{
		return mGraphics;
	}

	//======================================================= 

	// TODO(): This belongs in window class
	Enjon::Result Engine::ProcessInput( Enjon::Input* input, const f32 dt )
	{ 
		// Enable drop states for window
		SDL_EventState( SDL_DROPFILE, SDL_ENABLE );

	    SDL_Event event;
		Vec2 mouseWheel( 0.0f );
	   //Will keep looping until there are no more events to process
	    while ( SDL_PollEvent( &event ) ) 
	    { 
			mImGuiManager->ProcessEvent( &event );

			switch ( event.type )
			{
				case SDL_QUIT:
				{
					return Result::FAILURE;
				} break;

				case SDL_KEYUP:
				{
					input->ReleaseKey( event.key.keysym.sym );
				} break;

				case SDL_KEYDOWN:
				{
					input->PressKey( event.key.keysym.sym );
				} break;

				case SDL_MOUSEBUTTONDOWN:
				{
					input->PressKey( event.button.button );
				} break;

				case SDL_MOUSEBUTTONUP:
				{
					input->ReleaseKey( event.button.button );
				} break;

				case SDL_MOUSEMOTION:
				{
					input->SetMouseCoords( ( f32 )event.motion.x, ( f32 )event.motion.y );
				} break;

				case SDL_MOUSEWHEEL:
				{
					// Set mouse wheel for this frame
					mouseWheel = Vec2( event.wheel.x, event.wheel.y );
				} break;


			}

			// Pass event to windows
			for ( auto& w : mGraphics->GetWindows( ) )
			{
				w->ProcessInput( event );
			}

	    } 

		// Set mouse wheel this frame
		input->SetMouseWheel( mouseWheel );

	    return Result::PROCESS_RUNNING;
	}

	//======================================================= 

	Result EngineConfig::ParseArguments( s32 argc, char** argv )
	{ 
		// Parse arguments and place into config
		for ( s32 i = 0; i < argc; ++i )
		{
			String arg = String( argv[i] ); 

			// Set root path
			if ( arg.compare( "--enjon-path" ) == 0 && (i + 1) < argc )
			{
				mRootPath = String( argv[i + 1] ) + "/";
			}
			
			// Set root path
			if ( arg.compare( "--project-path" ) == 0 && (i + 1) < argc )
			{
				mProjectPath = String( argv[i + 1] );
			}
		} 

		// Make sure that root path is set for engine
		assert( ( mRootPath.compare( "" ) != 0 ) );

		return Result::SUCCESS;
	}

	//======================================================= 

	void EngineConfig::SetIsStandAloneApplication( bool enabled )
	{
		mIsStandalone = enabled;
	}

	//======================================================= 

	bool EngineConfig::IsStandAloneApplication( ) const
	{
		return mIsStandalone;
	}

	//======================================================= 

	void EngineConfig::SetRootPath( const String& path )
	{
		mRootPath = path;
	}

	//======================================================= 
			
	String EngineConfig::GetRoot() const
	{
		return mRootPath;
	}
	
	//======================================================= 
			
	String EngineConfig::GetEngineResourcePath() const
	{
		return mRootPath + "/Assets";
	}
	
	//======================================================= 

	f32 WorldTime::GetDeltaTime( )
	{
		return mDT;
	}
	
	//======================================================= 

	f32 WorldTime::GetTotalTimeElapsed( )
	{
		return mTotalTime;
	}
	
	//======================================================= 

	f32 WorldTime::GetAverageDeltaTime( )
	{
		return mAverageDT;
	}
	
	//======================================================= 

	f32 WorldTime::GetFPS( )
	{
		return mFPS;
	}

	//======================================================= 

	void WorldTime::CalculateAverageDeltaTime( )
	{
		static u32 tc = 10;
		static f32 mDTs[ 10 ];
		static u32 count = 0;
		static bool wrapped = false;

		mDTs[ count ] = mDT;
		count++;
		if ( count >= tc )
		{
			count = 0;
			wrapped = true;
		}

		if ( wrapped )
		{
			f32 sum = 0.0f;
			for ( u32 i = 0; i < tc; ++i )
			{
				sum += mDTs[ i ];
			}

			// Have wrapped, so calculate average
			sum /= ( f32 )tc;
			mAverageDT = sum;
		} 
	}
	
	//======================================================= 

	void Engine::BindImGuiContext( )
	{
		// Grab imguimanager 
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		// Bind the context to this memory
		igm->BindContext( );
	}
	
	//======================================================= 
}