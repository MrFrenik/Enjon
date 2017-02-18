#pragma once 
#ifndef ENJON_ENGINE_H
#define ENJON_ENGINE_H

namespace Enjon
{
	class Application;
	
	class Engine
	{
		public:
			Engine();
			~Engine();
			
			Engine* GetInstance();

		private:
			static Engine* mInstance;
			Application* mApp = nullptr;
	};
}

#endif