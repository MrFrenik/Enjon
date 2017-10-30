// @file UUID.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_UUID_H
#define ENJON_UUID_H

#include "System/Types.h"
#include "Defines.h"

#include <CrossGUID/guid.h>

namespace Enjon
{
	class UUID
	{
		public:

			/*
			* @brief Constructor
			*/
			UUID( );
			
			/*
			* @brief Constructor
			*/
			UUID( const Enjon::String& string );

			/*
			* @brief Destructor
			*/
			~UUID( );
			
			/*
			* @brief
			*/
			Enjon::String ToString();

			/*
			* @brief
			*/
			static UUID GenerateUUID( );

			/*
			* @brief
			*/
			static UUID Invalid( );

		private: 
			Guid mID;
	}; 
}

#endif
