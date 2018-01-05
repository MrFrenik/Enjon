// @file UUID.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_UUID_H
#define ENJON_UUID_H

#include "System/Types.h"
#include "Defines.h" 

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
			* @brief Constructor
			*/
			UUID( const u8* bytes );

			/*
			* @brief Destructor
			*/
			~UUID( );
			
			/*
			* @brief
			*/
			Enjon::String ToString() const; 

			/*
			* @brief
			*/
			explicit operator bool() const;

			/*
			* @brief
			*/
			static UUID GenerateUUID( );

			/*
			* @brief
			*/
			static UUID Invalid( );

			/*
			* @brief
			*/
			UUID &operator=( const UUID &other );

			/*
			* @brief
			*/
			bool operator==( const UUID &other ) const;

			/*
			* @brief
			*/
			bool operator!=( const UUID &other ) const;

		private:

			/*
			* @brief
			*/
			static UUID NewUUID( );
 
			/*
			* @brief
			*/
			friend std::ostream &operator<<( std::ostream &s, const UUID &uuid );
 
		private: 
			Vector<u8> mBytes; 
	}; 
}

#endif
