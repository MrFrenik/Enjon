#include "Engine.h"
#include "Application.h"

#include "SDL2/sdl.h"

#include <assert.h>

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

		return Enjon::Result::SUCCESS;
	}

	//-------------------------------------------------------
	Enjon::Result Engine::StartUp(Application* app)
	{
		StartUp();
		RegisterApplication(app);
	}

	//-------------------------------------------------------
	Engine* Engine::GetInstance()
	{
		return mInstance;
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
		static float dt = 0.0f;

		// Assert that application is registered with engine
		assert(mApp != nullptr);

		// Main application loop
		bool mIsRunning = true;
		while (mIsRunning)
		{
			// Update application
			mApp->Update(dt);

			// Increment dt
			dt += 0.01f;

			if (dt >= 1.0f) mIsRunning = false;
		}

		// Main engine loop here
		return Enjon::Result::SUCCESS;
	}
}