// Copyright 2016-2017, John Jackson. All Rights Reserved.  

#pragma once 
#ifndef ENJON_ENGINE_H
#define ENJON_ENGINE_H

#include "Defines.h"
#include "System/Types.h"

namespace Enjon
{
	class DeferredRenderer; 
	class Input;

	class Application; 
	
	class EngineConfig
	{
		public: 
		private: 
	};

	class Engine
	{
		public:
			Engine();
			~Engine();

			/**
			* @brief Called when first initializing engine. Runs through all subsystems and 
			*		initializes those as well
			* @return Result - Returns Success or Failure.
			*/
			Enjon::Result StartUp();

			/**
			* @brief Called when first initializing engine. Runs through all subsystems and 
			*		initializes those as well
			* @param app - Pointer to main application to be ran and to be registered with engine.
			* @return Result - Returns Success or Failure.
			*/
			Enjon::Result StartUp(Application* app);


			/**
			* @brief Begins main update loop of engine.
			* @return Result - Returns Success or Failure.
			*/
			Enjon::Result Run();
			
			/**
			* @brief Shuts down engine and all of its subsystems.
			* @return Result - Returns Success or Failure.
			*/
			Enjon::Result ShutDown();

			// NOTE(): I don't like this and will use a type catalog to grab 
			// registered subsystems "globally"
			/**
			* @brief Returns pointer to registered Graphics Subsystem.
			* @return DeferredRenderer* - Pointer to the registered graphics subsystem.
			*/
			DeferredRenderer* GetGraphics() { return mGraphics; }
			
			/**
			* @brief Returns pointer to registered Input Subsystem.
			* @return Input* - Pointer to the registered input subsystem.
			*/
			Input* GetInput() { return mInput; }

			/**
			* @brief Returns pointer to static instance of engine.
			* @return Engine* - Pointer to engine instance.
			*/
			static Engine* GetInstance();

		private:
			
			/**
			* @brief Sets the application pointer of the engine to the specified app.
			* @param app - The application to be registered
			* @return Result - Success or Failure on completion.
			*/
			Enjon::Result RegisterApplication(Application* app);

			/**
			* @brief Returns pointer to registered Input Subsystem.
			* @return InputManager* - Pointer to the registered input subsystem.
			*/
			Enjon::Result InitSubsystems();
			
			/**
			* @brief Processes and updates input system.
			* @return b8 - Returns true if success and false if failure.
			*/
			b8 ProcessInput(Input* input, float dt);

		private:

			static Engine* mInstance; 

			Application* mApp 				= nullptr; 
			DeferredRenderer* mGraphics 	= nullptr; 
			Input* mInput 					= nullptr;

	};
}

#endif