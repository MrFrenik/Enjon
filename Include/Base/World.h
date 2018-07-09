// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file World.h
#pragma once
#ifndef ENJON_WORLD_H
#define ENJON_WORLD_H

#include "System/Types.h"
#include "Defines.h" 
#include "Base/Object.h"

namespace Enjon
{
	class SubsystemContext;

	ENJON_CLASS( Abstract )
	class World : public Object
	{
		public: 

			/**
			* @brief
			*/
			World( ) = default;

			/**
			* @brief
			*/
			~World( );

			/**
			* @brief
			*/
			template < typename T >
			bool HasContext( )
			{
				const MetaClass* cls = Object::GetClass< T >( );
				if ( cls )
				{
					return ( mContextMap.find( cls->GetTypeId( ) ) != mContextMap.end( ) ); 
				}

				return false;
			}

			template < typename T >
			void RegisterContext( )
			{ 
				SubsystemContext::AssertIsSubsystemContext< T >( );
				if ( !HasContext< T >( ) )
				{
					const MetaClass* cls = Object::GetClass< T >( );
					// Construct new context and set world reference to this
					mContextMap[ cls->GetTypeId( ) ] = new T( this );
				}
			}

			template < typename T >
			T* GetContext( )
			{
				if ( HasContext< T >( ) )
				{
					return mContextMap[ Object::GetClass< T >( )->GetTypeId( ) ]->ConstCast< T > ( );
				}

				return nullptr;
			}

		protected: 
			HashMap< u32, SubsystemContext* > mContextMap;

	};
}

#endif

