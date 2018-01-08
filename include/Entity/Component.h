#pragma once
#ifndef ENJON_COMPONENT_H
#define ENJON_COMPONENT_H 

#include "Entity/EntityDefines.h"
#include "Math/Transform.h"
#include "System/Types.h"
#include "Base/Object.h"

#include <assert.h>
#include <array>
#include <vector>
#include <bitset>
#include <type_traits>
#include <unordered_map> 

namespace Enjon 
{ 
	// Forward declaration
	class Entity;
	class EntityManager;
	class Component;

	class ComponentWrapperBase
	{
		public:
			virtual void Base() = 0;

			virtual bool HasEntity( const u32& entityID ) = 0;

			virtual Component* AddComponent( const u32& entityId, Component* component ) = 0;

			virtual Component* AddComponent( const MetaClass* cls, const u32& entityId ) = 0;

			virtual void RemoveComponent( const u32& entityId ) = 0; 

			virtual Component* GetComponent( const u32& entityId ) = 0;

			virtual bool IsEmpty( ) const = 0;

			virtual u32 GetSize( ) const = 0;
	};

	class ComponentArray : public ComponentWrapperBase
	{
		friend Entity;
		friend EntityManager;
		friend Component;

		public:
			virtual void Base( ) override {}

			ComponentArray( );

			~ComponentArray( );

			using ComponentPtrs = Vector<Component*>; 
			using ComponentMap = HashMap<u32, Component*>;

			virtual Component* AddComponent( const u32& entityId, Component* component ) override
			{
				mComponentMap[entityId] = component;
				mComponentPtrs.push_back( mComponentMap[entityId] );
				return mComponentMap[entityId];
			}

			virtual Component* AddComponent( const MetaClass* cls, const u32& entityId ) override
			{
				// If not available then add component - otherwise return component that's already allocated
				if ( !HasEntity( entityId ) )
				{
					mComponentMap[entityId] = (Component*)cls->Construct( );
					mComponentPtrs.push_back( mComponentMap[entityId] );
				}

				return mComponentMap[entityId];
			}

			/**
			* @brief
			*/
			virtual void RemoveComponent( const u32& entityId ) override;

			virtual bool HasEntity( const u32& entityID ) override
			{
				return ( mComponentMap.find( entityID ) != mComponentMap.end( ) );
			}

			virtual Component* GetComponent( const u32& entityID ) override
			{
				if ( HasEntity( entityID ) )
				{
					return mComponentMap[entityID];
				}

				return nullptr;
			}

			virtual u32 GetSize( ) const override
			{
				return mComponentPtrs.size( );
			}

			virtual bool IsEmpty( ) const override
			{
				return ( GetSize( ) == 0 );
			}

		private:
			ComponentPtrs mComponentPtrs;
			ComponentMap mComponentMap; 
	};

	using ComponentID = u32;

	class Component : public Enjon::Object
	{
		friend Entity;
		friend EntityManager; 
		friend ComponentWrapperBase;
		friend ComponentArray;

		public:

			/**
			* @brief Constructor
			*/
			Component(){}

			virtual ~Component( )
			{ 
			}

			/**
			* @brief
			*/
			virtual void Initialize( );

			/**
			* @brief
			*/
			virtual void Update( const f32& dT );

			/**
			* @brief
			*/
			virtual void Shutdown( );

			/**
			* @brief
			*/
			Entity* GetEntity() const;

			/**
			* @brief
			*/
			u32 GetID() const { return mComponentID; }

			/**
			* @brief
			*/
			EntityManager* GetEntityManager() const; 

		protected:

			/**
			* @brief
			*/
			template <typename T>
			void DestroyBase()
			{
				assert( mBase != nullptr ); 
				mBase->RemoveComponent( mEntityID );
			}

			/**
			* @brief
			*/
			virtual void Destroy() = 0;

		private:
			/**
			* @brief
			*/
			void SetEntityManager( EntityManager* manager );

			/**
			* @brief
			*/
			void SetEntity( Entity* entity );

			/**
			* @brief
			*/
			void SetID( u32 id );

			/**
			* @brief
			*/
			void SetBase( ComponentWrapperBase* base );

			/**
			* @brief
			*/
			template <typename T>
			static inline ComponentID GetComponentType() noexcept
			{
				static_assert( std::is_base_of<Component, T>::value, "Component:: T must inherit from Component." );	
				return Engine::GetInstance( )->GetMetaClassRegistry( )->GetTypeId< T >( );
			}

		protected:
			Entity* mEntity = nullptr;
			EntityManager* mManager = nullptr;

			u32 mEntityID; 
			u32 mComponentID = 0;

		private:
			ComponentWrapperBase* mBase = nullptr;
	}; 



}

#endif
