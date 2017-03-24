#pragma once
#ifndef ENJON_APPLICATION_H
#define ENJON_APPLICATION_H

#include "System/Types.h"

namespace Enjon
{
	class Application
	{
		public:
			// Pure interface
			Application(){};
			~Application(){};

			/**
			* @brief Called when first initializing application. Runs through all startup code 
			*		that is necessary for the application to begin properly.
			* @return void
			*/
			virtual void Initialize() = 0;  

			/**
			* @brief Main update tick for application.
			* @return void
			*/
			virtual void Update(f32 dt) = 0;

			/**
			* @brief Shuts down application and cleans up any memory that was allocated.
			* @return void
			*/
			virtual void Shutdown() = 0; 
	};
}

#endif