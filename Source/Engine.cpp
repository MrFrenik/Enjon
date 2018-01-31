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
		// Shutdown all instances
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

	Enjon::Result Engine::InitSubsystems()
	{
		// Create new subsystem catalog
		mSubsystemCatalog = new SubsystemCatalog();

		// Meta class registration
		mMetaClassRegisty = new MetaClassRegistry( );

		// Register all base object meta classes
		Enjon::Object::BindMetaClasses( );

		// Register subsystems with catalog
		mAssetManager	= mSubsystemCatalog->Register<Enjon::AssetManager>( false );		// Will do manual initialization of asset management system, since it's project dependent
		mGraphics		= mSubsystemCatalog->Register<Enjon::GraphicsSubsystem>( );
		mInput			= mSubsystemCatalog->Register<Enjon::Input>( ); 
		mEntities		= mSubsystemCatalog->Register<EntityManager>( );
		mPhysics		= mSubsystemCatalog->Register<PhysicsSubsystem>( );

		// Initialize imgui manager
		Enjon::ImGuiManager::Init( mGraphics->GetWindow()->ConstCast< Window >()->GetSDLWindow() ); 

		// Initialize application if one is registered
		if ( mApp )
		{
			mApp->Initialize();
		} 

		// Initializes limiter
		 mLimiter.Init( 60.0f );

		// Late init for systems that need it
		Enjon::ImGuiManager::LateInit( mGraphics->GetWindow()->ConstCast< Window >()->GetSDLWindow() );

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
		static float dt = 0.01f;

		// Assert that application is registered with engine
		assert( mApp != nullptr ); 

		// Seed random 
		srand( time( NULL ) ); 

		// Main application loop
		bool mIsRunning = true;
		while (mIsRunning)
		{
			 mLimiter.Begin();

			 // Update input manager
			 mInput->Update( dt );

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

			// Update physics
			mPhysics->Update( dt );

			// Update graphics
			mGraphics->Update( dt );

			// Clamp frame rate
			//mLimiter.End(); 

			f32 fps = mLimiter.GetFPS( true );

			// Update world time
			mWorldTime.mDT = dt;
			mWorldTime.mTotalTime += mWorldTime.mDT;
			mWorldTime.mFPS = fps;
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

	GraphicsSubsystem* Engine::GetGraphicsSubsystem( ) const
	{
		return mGraphics;
	}

	//======================================================= 

	// TODO(): This belongs in window class
	Enjon::Result Engine::ProcessInput( Enjon::Input* input, const f32 dt )
	{
	    SDL_Event event;
	   //Will keep looping until there are no more events to process
	    while ( SDL_PollEvent( &event ) ) 
	    {
	    	ImGui_ImplSdlGL3_ProcessEvent( &event ); 

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
					input->SetMouseCoords( (f32)event.motion.x, (f32)event.motion.y ); 
				} break;

				case SDL_WINDOWEVENT: 
				{
					switch ( event.window.event )
					{
						case SDL_WINDOWEVENT_RESIZED: 
						{
							mGraphics->GetWindow( )->ConstCast< Window >()->SetViewport( iVec2( (u32)event.window.data1, (u32)event.window.data2 ) ); 
							mGraphics->ReinitializeRenderTargets( );
						}
						break; 

						case SDL_WINDOWEVENT_ENTER: 
						{

						}
						break;
						
						case SDL_WINDOWEVENT_LEAVE: 
						{

						}
						break;

						case SDL_WINDOWEVENT_MOVED:
						{

						}
						break;

						case SDL_WINDOWEVENT_RESTORED:
						{

						} break;

						case SDL_WINDOWEVENT_FOCUS_GAINED: 
						{

						} break;

						case SDL_WINDOWEVENT_FOCUS_LOST: 
						{

						} break;
						
						case SDL_WINDOWEVENT_CLOSE: 
						{

						} break; 
					}
				} break; 

				default:
				{

				} break;
			}
	    } 

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

}