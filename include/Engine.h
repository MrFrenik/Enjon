// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// File: Engine.h

#pragma once 
#ifndef ENJON_ENGINE_H
#define ENJON_ENGINE_H

#include "Defines.h"
#include "System/Types.h" 

namespace Enjon
{ 
	class SubsystemCatalog;
	class GraphicsSubsystem; 
	class Input;
	class AssetManager; 
	class Application; 
	class MetaClassRegistry;
	class EntityManager;
	class MetaClass;
	class Subsystem;
	
	class EngineConfig
	{
		public:
			EngineConfig() = default; 
			Result ParseArguments(s32 argc, char** argv); 

			String GetRoot() const; 
			String GetEngineResourcePath() const;

			void SetRootPath( const String& path ); 
			
		private:
			String mRootPath; 
			String mProjectPath;
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
			
			/**
			* @brief Returns pointer to registered Subsystem Catalog .
			* @return SubsystemCatalog* - Pointer to the registered SubsystemCatalog.
			*/
			SubsystemCatalog* GetSubsystemCatalog() { return mSubsystemCatalog; }
			
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
			
			/**
			* @brief Returns pointer to application.
			* @return Engine* - Pointer to engine instance.
			*/
			const Application* GetApplication( );
			
			/**
			* @brief Returns pointer to meta class registry.
			* @return MetaClassRegistry* - Pointer to meta class registry instance.
			*/
			const MetaClassRegistry* GetMetaClassRegistry( );

			/**
			* @brief
			*/
			static void SetInstance( Engine* instance )
			{
				if ( mInstance == nullptr )
				{
					mInstance = instance; 
				}
			}

			/**
			* @brief Returns pointer to registered requested subsystem by metaclass. Returns nullptr if not found.
			* @return Subsystem* - Pointer to requested subsystem
			*/
			const Subsystem* GetSubsystem( const MetaClass* cls ) const;

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
			Enjon::Result ProcessInput(Input* input, float dt);


		private:

			// Pointer to static instance of engine
			static Engine* mInstance; 

			// Main application
			Application*		mApp 				= nullptr; 

			// Subsystems
			GraphicsSubsystem*	mGraphics 			= nullptr; 
			Input*				mInput 				= nullptr;
			SubsystemCatalog*	mSubsystemCatalog	= nullptr;
			AssetManager*		mAssetManager		= nullptr;
			MetaClassRegistry*	mMetaClassRegisty	= nullptr;
			EntityManager*		mEntities			= nullptr;

			// Engine configuration settings
			EngineConfig mConfig;

	};
}

#endif