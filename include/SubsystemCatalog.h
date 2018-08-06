// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// @file: SubsystemCatalog.h
#pragma once
#ifndef ENJON_SUBSYSTEM_CATALOG_H
#define	ENJON_SUBSYSTEM_CATALOG_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Subsystem.h"
#include "Engine.h"

#include <unordered_map>

namespace Enjon
{
	class SubsystemCatalog : public Enjon::Object
	{
		public:

			/**
			*@brief Constructor
			*/
			SubsystemCatalog();

			/**
			*@brief Destructor
			*/
			~SubsystemCatalog(); 
			
			/**
			*@brief Registers with catalog as well as initializes subsystem
			*/
			template <typename T>
			T* Register(bool initialize = true)
			{
				static_assert( std::is_base_of<Subsystem, T>::value, 
					"SubsystemCatalog:: T must inherit from Subsystem." );	

				// Get idx of subsystem type
				u32 idx = GetSubsystemTypeId<T>();

				// Create new system
				T* system = new T();

				// Set into map
				mSubsystems[idx] = system;

				// Initialize system if set to true
				if ( initialize )
				{
					system->Initialize();
				} 
	
				return system;
			}

			/**
			*@brief Registers with catalog as well as initializes subsystem
			*@return The subsystem if registered, nullptr if not.
			*/
			template <typename T>
			const T* Get()
			{
				static_assert( std::is_base_of<Subsystem, T>::value, "SubsystemCatalog:: T must inherit from Subsystem." );	

				// Get idx of subsystem type
				u32 idx = GetSubsystemTypeId<T>(); 

				// If found, return the subsystem
				auto query = mSubsystems.find( idx );
				if ( query != mSubsystems.end() )
				{
					return static_cast<T*>(query->second);
				}

				return nullptr; 
			}

			/**
			* @brief
			*/
			bool Exists( u32& systemId ) const;

			/**
			* @brief Returns pointer to registered requested subsystem by metaclass. Returns nullptr if not found.
			* @return Subsystem* - Pointer to requested subsystem
			*/
			const Subsystem* Get( const MetaClass* cls ) const;

			/**
			*@brief Returns instance of this class
			*@return SubsystemCatalog* - Pointer to instance of this class
			*/
			SubsystemCatalog* GetInstance() const;

		private: 

			/**
			*@brief
			*/
			template <typename T>
			u32 GetSubsystemTypeId() noexcept
			{
				static_assert( std::is_base_of<Subsystem, T>::value,
					"GetSubsystemTypeId:: T must inherit from Subsystem." );

				return Engine::GetInstance( )->GetMetaClassRegistry( )->GetTypeId< T >( ); 
			} 

		private: 
			HashMap<u32, Subsystem*> mSubsystems; 
			static SubsystemCatalog* mInstance;
	};
}

#endif


