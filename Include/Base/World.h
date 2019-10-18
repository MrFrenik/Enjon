// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file World.h
#pragma once
#ifndef ENJON_WORLD_H
#define ENJON_WORLD_H

#include "System/Types.h"
#include "Defines.h" 
#include "Base/Object.h"
#include "Base/SubsystemContext.h"

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
		World( )
			: mShouldUpdate( true )
		{
		}

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
		T* RegisterContext( )
		{
			SubsystemContext::AssertIsSubsystemContext< T >( );
			const MetaClass* cls = Object::GetClass< T >( );
			if ( !HasContext< T >( ) )
			{
				// Construct new context and set world reference to this
				mContextMap[ cls->GetTypeId( ) ] = new T( this );
			}

			return (T*)mContextMap[ cls->GetTypeId() ];
		}

		template < typename T >
		T* GetContext( )
		{
			if ( HasContext< T >( ) )
			{
				return mContextMap[ Object::GetClass< T >( )->GetTypeId( ) ]->template ConstCast< T >( );
			}

			return nullptr;
		}

		template < typename T >
		void SetUpdates( bool updates )
		{
			if ( HasContext< T >( ) )
			{
				mContextMap[ Object::GetClass< T >( )->GetTypeId( ) ]->template ConstCast< T >( )->SetUpdates( updates );
			}
		}

		/*
		* @brief
		*/
		bool ShouldUpdate( ) const;

		/*
		* @brief
		*/
		void SetShouldUpdate( bool update );

	protected:
		HashMap< u32, SubsystemContext* > mContextMap;
		u32 mShouldUpdate : 1;
	};
}

#endif

