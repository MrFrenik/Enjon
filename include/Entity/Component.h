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
	template <typename T, typename U>
	constexpr size_t offsetof_impl( T const* t, U T::* a ) {
		return ( char const* )t - ( char const* )&( t->*a ) >= 0 ?
			( char const* )t - ( char const* )&( t->*a ) :
			( char const* )&( t->*a ) - ( char const* )t;
	}

	#define offset(Type_, Attr_)                          \
		offsetof_impl((Type_ const*)nullptr, &Type_::Attr_)

	template < typename T, typename V >
	usize Offset( V T::*mp )
	{
		return offsetof_impl< T, V >( ( T const* )nullptr, mp );
	}

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

	class Component : public Object
	{ 
		friend IComponentInstanceData;
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
			void SetID( u32 id );

			/**
			* @brief
			*/
			void SetEntityID( const u32& id );

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
	class IComponentRef : public Object
	{
		ENJON_CLASS_BODY( IComponentRef )

		protected: 
			const MetaClassComponent* mComponentMetaClass; 
	};

	class IComponentInstanceData;
	using PostConstructionComponentCallback = std::function< Result( const u32&, IComponentInstanceData* ) >;

	template < typename T >	
	struct ComponentHandle 
	{
		T* mComponent;
	};

	ENJON_CLASS( Abstract )
	class IComponentInstanceData : public Object
	{
		ENJON_CLASS_BODY( IComponentInstanceData )

		public: 

			virtual Component* GetComponentData( ) = 0; 
			virtual void Allocate( const u32& id ) = 0; 
			virtual void Deallocate( const u32& id ) = 0; 
			const virtual usize GetCount( ) = 0; 

			template < typename T >
			ComponentHandle< T >* GetComponentHandle( const u32& id )
			{
				ComponentHandle< Component >* it = GetComponentHandleInternal( id );
				return ( ComponentHandle< T >* )it; 
				//Component* data = GetComponentData( );
				//return ComponentHandle< T >{ data[ mEntityInstanceIndexMap[ id ] ] };
			} 

			virtual ComponentHandle< Component >* GetComponentHandleInternal( const u32& id ) = 0;

			template < typename T >
			static IComponentInstanceData* ConstructComponentInstanceData( )
			{
				return new ComponentInstanceData< T >( );
			}

			bool HasComponent( const u32& id )
			{
				return mEntityInstanceIndexMap.find( id ) != mEntityInstanceIndexMap.end( );
			} 

			const Vector< u32 >& GetEntityIDs( )
			{
				return mEntityID_InstanceData;
			}

			void RegisterPostConstructionCallback( const PostConstructionComponentCallback& cb )
			{
				mPostConstructionCallbacks.push_back( cb );
			}

		protected:

			void PostComponentConstruction( const u32& id )
			{
				for ( auto& f : mPostConstructionCallbacks )
				{
					f( id, this );
				}
			}

		protected:
			Vector< u32 > mEntityID_InstanceData;
			HashMap< u32, u32 > mEntityInstanceIndexMap; 
			HashMap< u32, void* > mDataIndexPtr;
			Vector< PostConstructionComponentCallback > mPostConstructionCallbacks;
	};

	template < typename T >
	class ComponentInstanceData : public IComponentInstanceData
	{ 
		public: 
			ComponentInstanceData( ) = default;

			virtual Component* GetComponentData( ) override
			{
				return mData.data( );
			}

			const virtual usize GetCount( ) override
			{
				return mData.size( );
			}

			virtual void Allocate( const u32& id ) override
			{
				if ( !HasComponent( id ) )
				{
					// Push back new instance
					T cmp;
					cmp.SetEntityID( id ); 
					mData.push_back( cmp );

					// Push back entity id ( Don't think I need to do this anymore )
					mEntityID_InstanceData.push_back( id );

					// Update entity index map
					mEntityInstanceIndexMap[ id ] = ( mData.size() - 1 );

					// Add component handle
					mComponentHandles[ id ] = ComponentHandle< T > { &mData.at( mData.size( ) - 1 ) };

					// Post construction on component data
					PostComponentConstruction( id );
				}
			} 

			virtual void Deallocate( const u32& id ) override
			{
				if ( HasComponent( id ) )
				{
					// Grab component index from given entity id
					u32 idx = mEntityInstanceIndexMap[ id ];

					// Get size of data arrays
					u32 dataCount = GetCount( );

					// Swap and pop all data ( slowest part of this entire operation )
					if ( dataCount > 1 )
					{
						std::iter_swap( mData.begin( ) + idx, mData.end( ) - 1 );

						// Swap entity id instance as well
						std::iter_swap( mEntityID_InstanceData.begin( ) + idx, mEntityID_InstanceData.end( ) - 1 );

						// Swap component handle pointers
					}
					bool idxIsLast = ( idx == ( dataCount - 1 ) );

					// Pop all data arrays
					mData.pop_back( );

					// Pop entity instance id data as well
					mEntityID_InstanceData.pop_back( );

					// Erase entity id from index map
					mEntityInstanceIndexMap.erase( id );

					// Swap around index indirection
					if ( !mData.empty( ) && !idxIsLast )
					{
						mEntityInstanceIndexMap[ mEntityID_InstanceData.at( idx ) ] = idx;
					}

					// Remove component handle
					mComponentHandles.erase( id ); 
				}
			}

			virtual ComponentHandle< Component >* GetComponentHandleInternal( const u32& id ) override
			{
				return ( ComponentHandle< Component >* )( &mComponentHandles[ id ] );
			}

			Vector< T > mData; 
			HashMap< u32, ComponentHandle< T > > mComponentHandles;
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
