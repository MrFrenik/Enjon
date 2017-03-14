#include "Engine.h"
#include "Application.h"
#include "Graphics/DeferredRenderer.h"
#include "IO/InputManager.h"
#include "ImGui/ImGuiManager.h"
#include "Utils/Timing.h"

#include "SDL2/sdl.h"

#include <assert.h>
#include <random>
#include <time.h>

// Totally temporary
static bool mMovementOn = false;

// Enjon::Utils::FPSLimiter mLimiter;

namespace Enjon
{
	Engine* Engine::mInstance = nullptr;

	//-------------------------------------------------------
	Engine::Engine()
	{
		assert(mInstance == nullptr);
		mInstance = this;
	}	

	//-------------------------------------------------------
	Engine::~Engine()
	{
		// Shutdown all instances
	}

	//-------------------------------------------------------
	Enjon::Result Engine::StartUp()
	{
		// TODO(): Find out where this should be abstracted into
		 //Initialize SDL
		SDL_Init(SDL_INIT_EVERYTHING);
		
		//Tell SDL that we want a double buffered window so we dont get any flickering
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

		InitSubsystems();

		return Enjon::Result::SUCCESS;
	}

	//-------------------------------------------------------
	Enjon::Result Engine::StartUp(Application* app)
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
		res = StartUp();
		if (res != Enjon::Result::SUCCESS)
		{
			// Error here
			assert(false);
			return res;
		}

		// Return result
		return res;
	}

	//-------------------------------------------------------
	Engine* Engine::GetInstance()
	{
		return mInstance;
	}

	Enjon::Result Engine::InitSubsystems()
	{
		// Construct new graphics engine and initialize
		mGraphics = new Enjon::Graphics::DeferredRenderer();	
		mGraphics->Init();

		// Construct new input manager
		mInput = new Enjon::Input::InputManager();

		// Initialize imgui manager
		Enjon::ImGuiManager::Init(mGraphics->GetWindow()->GetSDLWindow());

		// Initialize application if one is registered
		if (mApp)
		{
			mApp->Initialize();
		}

		// mLimiter.Init(60.0f);

		// Late init for systems that need it
		Enjon::ImGuiManager::LateInit(mGraphics->GetWindow()->GetSDLWindow());

		return Enjon::Result::SUCCESS;
	}

	//-------------------------------------------------------
	Enjon::Result Engine::RegisterApplication(Application* app)
	{
		assert(mApp == nullptr);
		mApp = app;

		return Enjon::Result::SUCCESS;
	}

	Enjon::Result Engine::Run()
	{
		static float dt = 0.1f;

		// Assert that application is registered with engine
		assert(mApp != nullptr);

		// Seed random 
		srand(time(NULL));

		// Main application loop
		bool mIsRunning = true;
		while (mIsRunning)
		{
			// mLimiter.Begin();

			mInput->Update();

			// Update input
			mIsRunning = ProcessInput(mInput, dt);

			// Update application
			mApp->Update(dt);

			// Update graphics
			mGraphics->Update(dt);

			// mLimiter.End();
		}

		// Main engine loop here
		return Enjon::Result::SUCCESS;
	}

	Enjon::Result Engine::ShutDown()
	{
		if (mApp)
		{
			mApp->Shutdown();
		}

		return Enjon::Result::SUCCESS;
	}

	bool Engine::ProcessInput(EI::InputManager* input, float dt)
	{
	    SDL_Event event;
	   //Will keep looping until there are no more events to process
	    while (SDL_PollEvent(&event)) 
	    {
	    	ImGui_ImplSdlGL3_ProcessEvent(&event);

	        switch (event.type) 
	        {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					input->ReleaseKey(event.key.keysym.sym); 
					break;
				case SDL_KEYDOWN:
					input->PressKey(event.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
				default:
					break;
			}
	    }

	    if (input->IsKeyPressed(SDLK_ESCAPE))
	    {
	    	return false;
	    }

	    

	    return true;
	}
}