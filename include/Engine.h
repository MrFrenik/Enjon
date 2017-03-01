#pragma once 
#ifndef ENJON_ENGINE_H
#define ENJON_ENGINE_H

#include "Defines.h"

namespace Enjon
{
	namespace Graphics
	{
		class DeferredRenderer;
	}

	namespace Input
	{
		class InputManager;
	}

	class Application;

	class Engine
	{
		public:
			Engine();
			~Engine();
			
			Enjon::Result StartUp();
			Enjon::Result StartUp(Application* app);
			Enjon::Result RegisterApplication(Application* app);
			Enjon::Result Run();
			Enjon::Result ShutDown();

			// NOTE(): I don't like this and will use a type catalog to grab 
			// registered subsystems "globally"
			Graphics::DeferredRenderer* GetGraphics() { return mGraphics; }

			static Engine* GetInstance();

		private:
			Enjon::Result InitSubsystems();
			bool ProcessInput(Input::InputManager* input, float dt);

		private:
			static Engine* mInstance;

			Application* mApp 						= nullptr;
			Graphics::DeferredRenderer* mGraphics 	= nullptr;
			Input::InputManager* mInput 			= nullptr;

	};
}

#endif