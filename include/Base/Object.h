// @file Object.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_OBJECT_H
#define ENJON_OBJECT_H 

#include "MetaClass.h"
#include "System/Types.h"
#include "Engine.h"
#include "Defines.h"

#include <limits>
#include <assert.h>

#define ENJON_OBJECT( type )																		\
	friend Object;																					\
	public:																							\
		virtual u32 GetTypeId() const override { return Enjon::Object::GetTypeId< type >(); }		\
		virtual const char* GetTypeName() const override { return #type; }							\
		virtual const MetaClass* type::Class( ) override\
		{\
			MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance()->GetMetaClassRegistry( ) );\
			const MetaClass* cls = mr->Get< type >( );\
			if ( !cls )\
			{\
				cls = mr->RegisterMetaClass< type >( );\
			}\
			return cls;\
		}

#define ENJON_PROPERTY(...)
#define ENJON_FUNCTION(...)

namespace Enjon
{
	enum class MetaPropertyType
	{
		Unknown,
		Bool,
		ColorRGBA16,
		Float_32,
		Float_64,
		Uint_8,
		Uint_16,
		Uint_32,
		Uint_64,
		Int_8,
		Int_16,
		Int_32,
		Int_64,
		String,
		Array,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
		Quat,
		Enum,
		UUID
	};

	class MetaClass;
	class Object;

	class MetaProperty
	{
		friend MetaClass;
		friend Object;
	public:
		MetaProperty( ) = default;
		MetaProperty( MetaPropertyType type, const std::string& name, u32 offset )
			: mType( type ), mName( name ), mOffset( offset )
		{
		}

		~MetaProperty( )
		{
		}

		std::string GetName( );

		MetaPropertyType GetType( );

	protected:
		MetaPropertyType mType;
		std::string mName;
		u32 mOffset;
	};

	typedef std::unordered_map< std::string, MetaProperty > PropertyTable;
	typedef std::unordered_map< std::string, MetaFunction > FunctionTable;

	class MetaClass
	{
		friend Object;

	public:
		MetaClass( ) = default;
		~MetaClass( )
		{
		}

		u32 PropertyCount( ) const
		{
			return ( u32 )mProperties.size( );
		}

		u32 FunctionCount( )
		{
			return ( u32 )mFunctions.size( );
		}

		bool PropertyExists( const std::string& propertyName )
		{
			return ( mProperties.find( propertyName ) != mProperties.end( ) );
		}

		const MetaProperty* GetProperty( const std::string& propertyName )
		{
			if ( PropertyExists( propertyName ) )
			{
				return &mProperties[ propertyName ];
			}
			return nullptr;
		}

		template < typename T >
		void GetValue( const Object* obj, const MetaProperty* property, T* value )
		{
			if ( PropertyExists( property->mName ) )
			{
				void* member_ptr = ( ( ( u8* )&( *obj ) + property->mOffset ) );
				*value = *( T* )member_ptr;
			}
		}

		template < typename T >
		void GetValue( const Object* obj, const std::string& propertyName, T* value )
		{
			if ( PropertyExists( propertyName ) )
			{
				MetaProperty* prop = &mProperties[ propertyName ];
				void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
				*value = *( T* )member_ptr;
			}
		}

		template < typename T >
		void SetValue( const Object* obj, const std::string& propertyName, const T& value )
		{
			if ( PropertyExists( propertyName ) )
			{
				MetaProperty* prop = &mProperties[ propertyName ];
				void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
				*( T* )( member_ptr ) = value;
			}
		}

		template < typename T >
		void SetValue( const Object* obj, const MetaProperty* prop, const T& value )
		{
			if ( PropertyExists( prop->mName ) )
			{
				void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
				*( T* )( member_ptr ) = value;
			}
		}

		PropertyTable* GetProperties( ) { return &mProperties; }

	protected:
		PropertyTable mProperties;
		FunctionTable mFunctions;
	};

	class Object;
	class MetaClassRegistry
	{
		public:
			MetaClassRegistry( )
			{
			}

			~MetaClassRegistry( )
			{
			}

			template <typename T>
			MetaClass* RegisterMetaClass( )
			{
				// Must inherit from object to be able to registered
				static_assert( std::is_base_of<Object, T>::value, "RegisterMetaClass:: T must inherit from Object." );

				// Get id of object
				u32 id = Object::GetTypeId<T>( );

				// If available, then return
				if ( HasMetaClass< T >( ) )
				{
					return mRegistry[ id ];
				}

				// Otherwise construct it and return
				MetaClass* cls = Object::ConstructMetaClass< T >( );

				mRegistry[ id ] = cls;
				return cls;
			}

			template < typename T >
			bool HasMetaClass( )
			{
				return ( mRegistry.find( Object::GetTypeId< T >( ) ) != mRegistry.end( ) );
			}

			template < typename T >
			const MetaClass* Get( )
			{
				return HasMetaClass< T >( ) ? mRegistry[ Object::GetTypeId< T >( ) ] : nullptr;
			}

		private:
			std::unordered_map< u32, MetaClass* > mRegistry;
	};

	// Max allowed type id by engine
	const u32 EnjonMaxTypeId = std::numeric_limits<u32>::max( ) - 1;

	// Base model for all Enjon classes that participate in reflection
	class Object
	{
		friend MetaClassRegistry;

		public:

			/**
			*@brief
			*/
			Object( ) {}

			/**
			*@brief
			*/
			~Object( ) {}

			/**
			*@brief
			*/
			virtual const MetaClass* Class( ) = 0;

			/**
			*@brief
			*/
			virtual u32 GetTypeId( ) const = 0;

			/**
			*@brief
			*/
			virtual const char* GetTypeName( ) const = 0;

			/**
			*@brief
			*/
			template <typename T>
			T* Cast( )
			{
				static_assert( std::is_base_of<Object, T>::value, "Cast:: T must inherit from Enjon Object." );

				return static_cast<T*>( this );
			}

			/**
			*@brief
			*/
			template <typename T>
			static u32 GetTypeId( ) noexcept
			{
				static_assert( std::is_base_of<Object, T>::value, "GetTypeId:: T must inherit from Enjon Object." );

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

		protected:
			template <typename T>
			static MetaClass* ConstructMetaClass( );

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

