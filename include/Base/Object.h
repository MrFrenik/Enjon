// @file Object.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_OBJECT_H
#define ENJON_OBJECT_H 

#include "System/Types.h"
#include "Defines.h"

#include <limits>
#include <assert.h>

#define ENJON_OBJECT( type )																		\
	public:																							\
		virtual u32 GetTypeId() const override { return Enjon::Object::GetTypeId< type >(); }		\
		virtual const char* GetTypeName() const override { return #type; } 

#define ENJON_PROPERTY(...)
#define ENJON_FUNCTION(...)

namespace Enjon
{
	// Max allowed type id by engine
	const u32 EnjonMaxTypeId = std::numeric_limits<u32>::max( ) - 1;

	// Base model for all Enjon classes that participate in reflection
	class Object
	{
	public:

		/**
		*@brief
		*/
		Object( ) {}

		/**
		*@brief
		*/
		~Object( ) {}

		virtual u32 GetTypeId( ) const = 0;
		
		virtual const char* GetTypeName( ) const = 0;

		/**
		*@brief
		*/
		template <typename T>
		T* Cast( )
		{
			return static_cast<T*>( this );
		}

		/**
		*@brief
		*/
		template <typename T>
		static u32 GetTypeId( ) noexcept
		{
			static_assert( std::is_base_of<Object, T>::value,
			"GetTypeId:: T must inherit from Enjon Object." );

			static u32 typeId { GetUniqueTypeId( ) };
			return typeId;
		}

		template < typename T >
		bool InstanceOf( )
		{
			return this->GetTypeId( ) == GetTypeId< T >( );
		}

		/**
		*@brief
		*/
		bool TypeValid( ) const; 

	protected:

		// Default to u32 max; If id set to this, then is not set by engine and invalid
		u32 mTypeId = 0;

	private:

		/**
		*@brief
		*/
		static u32 GetUniqueTypeId( ) noexcept
		{
			static u32 lastId { 1 };
			return lastId++;
		} 
	};
}


#endif

