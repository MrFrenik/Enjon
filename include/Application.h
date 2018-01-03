// @file Application.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once 
#ifndef ENJON_APPLICATION_H
#define ENJON_APPLICATION_H

#include "System/Types.h"
#include "Base/Object.h"
#include "Defines.h"

namespace Enjon
{
	class Application : public Enjon::Object
	{
		public:
			/**
			* @brief Constructor
			*/
			Application() = default; 

			/**
			* @brief Destructor
			*/
			~Application() = default;

			/**
			* @brief Called when first initializing application. Runs through all startup code 
			*		that is necessary for the application to begin properly.
			* @return Enjon::Result
			*/
			virtual Enjon::Result Initialize() = 0;  

			/**
			* @brief Main update tick for application.
			* @return Enjon::Result
			*/
			virtual Enjon::Result Update(f32 dt) = 0;

			/**
			* @brief Processes input from input class 
			* @return Enjon::Result
			*/
			virtual Enjon::Result ProcessInput(f32 dt);

			/**
			* @brief Shuts down application and cleans up any memory that was allocated.
			* @return Enjon::Result
			*/
			virtual Enjon::Result Shutdown() = 0; 
			
			/**
			* @brief Returns name of application
			* @return Enjon::String
			*/
			const Enjon::String& GetApplicationName( )
			{
				return mApplicationName;
			}

		protected:
			Enjon::String mApplicationName = "";
	};
}

#endif