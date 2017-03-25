// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// File: Engine.h

#pragma once 
#ifndef ENJON_ENGINE_H
#define ENJON_ENGINE_H

#include "Defines.h"
#include "System/Types.h" 

namespace Enjon
{
	using String = std::string; 

	class DeferredRenderer; 
	class Input;

	class Application; 
	
	// What can engine config hold?
	// A root path for the engine
	class EngineConfig
	{
		public:
			EngineConfig() = default; 
			Result ParseArguments(s32 argc, char** argv); 

			const String& GetRoot() const;
			
		private:
			String mRootPath; 
	};

	class Engine
	{
		public:
			Engine();
			~Engine();

			/**
			* @brief Called when first initializing engine. Runs through all subsystems and 
			*		initializes those as well
			* @param config - Configuration for engine.
			* @return Result - Returns Success or Failure.
			*/
			Enjon::Result StartUp(const EngineConfig& config);

			/**
			* @brief Called when first initializing engine. Runs through all subsystems and 
			*		initializes those as well
			* @param app - Pointer to main application to be ran and to be registered with engine.
			* @param config - Configuration for engine.
			* @return Result - Returns Success or Failure.
			*/
			Enjon::Result StartUp(Application* app, const EngineConfig& config);


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
			* @brief Returns const reference to engine config.
			* @return const EngineConfig& - Const reference to the registered engine config.
			*/
			const EngineConfig& GetConfig() const;

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
			EngineConfig mConfig;

	};
}

#endif