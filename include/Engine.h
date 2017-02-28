#pragma once 
#ifndef ENJON_ENGINE_H
#define ENJON_ENGINE_H

#include "Defines.h"

namespace Enjon
{
	class Application;

	class Engine
	{
		public:
			Engine();
			~Engine();
			
			Enjon::Result StartUp();
			Enjon::Result StartUp(Application* app);
			Engine* GetInstance();
			Enjon::Result RegisterApplication(Application* app);
			Enjon::Result Run();

		private:
			static Engine* mInstance;
			Application* mApp = nullptr;
	};
}

#endif