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

	using ComponentID = u32;

	enum class ComponentTickState
	{
		TickAlways,
		TickOnRunOnly,
		TickNever 
	};

	class ComponentWrapperBase
	{
		public:
			virtual void Base() = 0;

			virtual ~ComponentWrapperBase( )
			{ 
			}

			virtual bool HasEntity( const u32& entityID ) = 0;

			virtual Component* AddComponent( const u32& entityId, Component* component ) = 0;

			virtual Component* AddComponent( const MetaClass* cls, const u32& entityId ) = 0;

			virtual void RemoveComponent( const u32& entityId ) = 0; 

			virtual Component* GetComponent( const u32& entityId ) = 0;

			virtual bool IsEmpty( ) const = 0;

			virtual u32 GetSize( ) const = 0;

			virtual Vector<Component*> GetComponents( ) = 0;

			virtual void Update( ) = 0;

			virtual ComponentTickState GetTickState( ) = 0;
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

			/**
			* @brief
			*/
			virtual void Update( ) override;

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

			virtual Vector<Component*> GetComponents( ) override
			{
				return mComponentPtrs;
			}

			virtual ComponentTickState GetTickState( ) override
			{
				return mTickState;
			}

		private:
			ComponentPtrs mComponentPtrs;
			ComponentMap mComponentMap; 
			ComponentTickState mTickState;
	};

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
			* @brief Called immediately after constructing the component. Can assume entity ownership at this point however cannot assume that any components are available for reference.
			*/
			virtual void PostConstruction( );

			/**
			* @brief Called once per life-cycle of each component BEFORE Start() is called. Not safe to assume any other components are initialized or registered.
			*/
			virtual void Initialize( );

			/**
			* @brief Called once per life-cycle of each component. It is safe to assume at this point that all other components are initailized and registered.
			*/
			virtual void Start( );

			/**
			* @brief Called once per frame on each component.
			*/
			virtual void Update( );

			/**
			* @brief Called at the end of the component life-cycle. Any state or memory that needs to be cleaned up with happen here.
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
			ComponentTickState GetTickState( ) const
			{
				return mTickState;
			}

			virtual void AddToWorld( World* world )
			{ 
			} 

			virtual void RemoveFromWorld( )
			{ 
			}

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

			/*
			* @brief
			*/
			virtual void UpdateTransform( const Transform& transform );

		private:

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

			ComponentTickState mTickState = ComponentTickState::TickOnRunOnly;

			u32 mEntityID; 
			u32 mComponentID = 0;

		private:
			ComponentWrapperBase* mBase = nullptr; 
	}; 

	ENJON_CLASS( Abstract )
	class IComponentSystem : public Object
	{
		ENJON_CLASS_BODY( IComponentSystem )

		public: 

			/*
			* @brief
			*/
			virtual void Update( ) = 0;

			/*
			* @brief
			*/
			const ComponentTickState GetTickState( )
			{
				return mTickState;
			}

		protected:
			ComponentTickState mTickState = ComponentTickState::TickAlways;
	};



}

#endif
