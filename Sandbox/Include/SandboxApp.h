#ifndef ENJON_SANDBOX_APP_H
#define ENJON_SANDBOX_APP_H

#include <System/Types.h>
#include <Defines.h>
#include <Application.h>

namespace Enjon
{
	ENJON_CLASS( Construct )
	class SandboxApp : public Application
	{ 
		ENJON_MODULE_BODY( SandboxApp )

		public: 

			SandboxApp( const String& projectPath );

			/**
			* @brief Called when first initializing application. Runs through all startup code 
			*		that is necessary for the application to begin properly.
			* @return Enjon::Result
			*/
			virtual Result Initialize() override;  

			/**
			* @brief Main update tick for application.
			* @return Enjon::Result
			*/
			virtual Result Update(f32 dt) override;

			/**
			* @brief Processes input from input class 
			* @return Enjon::Result
			*/
			virtual Result ProcessInput(f32 dt) override;

			/**
			* @brief Shuts down application and cleans up any memory that was allocated.
			* @return Enjon::Result
			*/
			virtual Result Shutdown() override; 

		protected:

			void InitializeProjectApp();

		protected:

			/*
			* @brief The application to run during the shell
			*/
			Application* mApp = nullptr;

			/*
			* @brief Directory of the project 
			*/
			String mProjectDir = "";
	};

	// Declaration for module export
	ENJON_MODULE_DECLARE( SandboxApp ) 
} 

#endif


