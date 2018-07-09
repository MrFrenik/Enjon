// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file SubsystemContext.h
#pragma once
#ifndef ENJON_SUBSYSTEM_CONTEXT_H
#define ENJON_SUBSYSTEM_CONTEXT_H

#include "System/Types.h"
#include "Defines.h" 
#include "Base/Object.h"

namespace Enjon
{
	class World;

	ENJON_CLASS( Abstract )
	class SubsystemContext : public Enjon::Object
	{ 
		public:

			/**
			* @brief
			*/
			SubsystemContext( ) = default;

			/**
			* @brief
			*/
			SubsystemContext( World* world );

			/**
			* @brief
			*/
			virtual ~SubsystemContext( ) = default;

			template <typename T>
			inline static void AssertIsSubsystemContext( )
			{
				static_assert( std::is_base_of<SubsystemContext, T>::value, "T must inherit from SubsystemContext." ); 
			} 

		protected:

			World* mWorld = nullptr;
	}; 
}

#endif