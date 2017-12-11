// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// @file: SubsystemCatalog.h
#pragma once
#ifndef ENJON_SUBSYSTEM_CATALOG_H
#define	ENJON_SUBSYSTEM_CATALOG_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Subsystem.h"

#include <unordered_map>

namespace Enjon
{
	class SubsystemCatalog
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

				// Initialize system if set to true
				if ( initialize )
				{
					system->Initialize();
				}
				
				// Set into map
				mSubsystems[idx] = system;
	
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
			*@brief Returns instance of this class
			*@return SubsystemCatalog* - Pointer to instance of this class
			*/
			SubsystemCatalog* GetInstance() const;

		private:

			/**
			*@brief
			*/
			u32 GetUniqueSubsystemTypeId() noexcept
			{
				static u32 lastId{ 0u };
				return lastId++;
			} 

			/**
			*@brief
			*/
			template <typename T>
			u32 GetSubsystemTypeId() noexcept
			{
				static_assert( std::is_base_of<Subsystem, T>::value,
					"GetSubsystemTypeId:: T must inherit from Subsystem." );

				static u32 typeId{ GetUniqueSubsystemTypeId() };
				return typeId;
			} 

		private: 
			std::unordered_map<u32, Subsystem*> mSubsystems; 
			static SubsystemCatalog* mInstance;
	};
}

#endif


