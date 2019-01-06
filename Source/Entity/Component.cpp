#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "Base/World.h"
#include "Engine.h"
#include "Application.h"

namespace Enjon 
{ 
	
	//=========================================================================

	ComponentArray::ComponentArray( )
	{ 
	}
	
	//=========================================================================

	ComponentArray::~ComponentArray( )
	{
		mComponentMap.clear( );
		mComponentPtrs.clear( );
	}

	//=========================================================================

	Entity* Component::GetEntity() const
	{
		return EngineSubsystem( EntityManager )->GetRawEntity( mEntityID );
	}
	
	//=========================================================================

	void Component::SetEntity( Entity* entity )
	{
		mEntity = entity;
	}
	
	//=========================================================================

	void Component::SetID( u32 id )
	{
		mComponentID = id;
	}
	
	//=========================================================================

	void Component::SetEntityID( const u32& id )
	{
		mEntityID = id;
	}

	void Component::SetBase( ComponentWrapperBase* base )
	{
		mBase = base;
	} 
	
	//=========================================================================

	void Component::PostConstruction( )
	{ 
		// Nothing by default...
	}
	
	//=========================================================================

	void Component::Initialize( )
	{ 
		// Nothing by default...
	}
	
	//=========================================================================

	void Component::Start( )
	{ 
		// Nothing by default...
	}
	
	//=========================================================================

	void Component::Update( )
	{ 
		// Nothing by default...
	} 
	
	//=========================================================================

	void Component::Shutdown( )
	{ 
		// Nothing by default
	}
	
	//=========================================================================

	void ComponentArray::RemoveComponent( const u32& entityId )
	{
		if ( HasEntity( entityId ) )
		{
			auto cPtrList = &mComponentPtrs;
			auto cMap = &mComponentMap;

			// Get component
			auto compPtr = mComponentMap[entityId];

			// Remove ptr from point list map
			cPtrList->erase( std::remove( cPtrList->begin( ), cPtrList->end( ), compPtr ), cPtrList->end( ) );

			// Finally remove from map
			cMap->erase( entityId );

			// Set component's base to nullptr
			compPtr->mBase = nullptr;
		}
	} 
	
	//=========================================================================

	void Component::UpdateTransform( const Transform& transform )
	{
		// Nothing by default...
	}
	
	//=========================================================================

	void ComponentArray::Update( )
	{
		// Get the application
		const Application* app = Engine::GetInstance( )->GetApplication( );
 
		// Update all components
		for ( auto& c : mComponentPtrs )
		{
			mTickState = c->GetTickState( );

			if ( c->GetTickState() == ComponentTickState::TickAlways || app->GetApplicationState( ) == ApplicationState::Running )
			{
				Entity* ent = c->GetEntity( );
				if ( ent->GetWorld()->ShouldUpdate() && ent->GetState( ) == EntityState::ACTIVE )
				{
					c->Update( ); 
				}
			}
			// Otherwise kill loop and return ( Note: not cache friendly, but oh wells )
			else
			{
				return;
			}
		} 
	} 

	//=========================================================================
}




// ECS Sketch Ideas
/*

ENJON_COMPONENT_DECL()
class HealthComponent : public IComponent
{
	ENJON_COMPONENT_DECL_BODY( HealthComponent )

	public:

		ENJON_PROP()
		f32 mHealth;

		ENJON_PROP()
		f32 mRegenRate;
};
 
// Precompiler takes this decl and then generates an InstanceData structure from it
class HealthComponent_InstanceData : public IComponentInstanceData
{
	Vector< f32 > mHealth;
	Vector< f32 > mRegenRate;
};

class IComponentDataManager
{
	virutal void Base() = 0;
};

template < typename T >
class ComponentDataManager : public IComponentDataManager
{
	virtual void Base() override = {}

	protected:
		T mInstanceData;	
		HashMap< u32, u32 > mIndexMap;
};

// How do you access data? How do you set data? 
// Don't feel comfortable grabbing the entire data block for the user to set individual pieces of data

// Generated class information
class HealthComponentDataManager : public ComponentDataManager< HealthComponent_InstanceData >
{
			
};

class EntityManager
{
	//...

	private:

		HashMap< u32, IComponentDataManager* > mComponentDataManagers;
};


















*/



