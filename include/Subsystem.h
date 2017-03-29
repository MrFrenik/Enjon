// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file Subsystem.h
#pragma once
#ifndef ENJON_SUBSYSTEM_H
#define ENJON_SUBSYSTEM_H

#include "System/Types.h"
#include "Defines.h"

namespace Enjon
{
	class Subsystem
	{
		public: 

			/**
			*@brief Constructor
			*/
			// TODO(): Add variadic template arguments for this
			Subsystem() = default;

			/**
			*@brief Constructor
			*/
			~Subsystem() {}
			
			/**
			*@brief
			*/
			virtual Enjon::Result Initialize() = 0;

			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) = 0;
			
			/**
			*@brief
			*/
			virtual Enjon::Result Shutdown() = 0;

		protected:

		private: 

	};
}

#endif
