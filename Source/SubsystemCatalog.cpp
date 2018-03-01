// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// @file: SubsystemCatalog.cpp

#include "SubsystemCatalog.h"
#include "Asset/AssetManager.h"
#include "Physics/PhysicsSubsystem.h"
#include "Entity/EntityManager.h"
#include "Graphics/GraphicsSubsystem.h"
#include "IO/InputManager.h"

#include <assert.h>

namespace Enjon
{ 
	SubsystemCatalog* SubsystemCatalog::mInstance = nullptr;

	//======================================================================================================

	SubsystemCatalog::SubsystemCatalog()
	{
		assert(mInstance == nullptr);
		mInstance = this; 
	}
	
	//======================================================================================================

	SubsystemCatalog::~SubsystemCatalog()
	{ 
		// Shutdown individual subsystems
		// NOTE(): Need to explicit in ordering of shutdowns to prevent crashes!

		EngineSubsystem( EntityManager )->Shutdown( ); 
		EngineSubsystem( AssetManager )->Shutdown( );
		EngineSubsystem( Input )->Shutdown( );
		EngineSubsystem( PhysicsSubsystem )->Shutdown( ); 
		EngineSubsystem( GraphicsSubsystem )->Shutdown( );

		// Delete all subsystems to clear memory
		// NOTE(): No subsystem should have an explicit destructor! Not safe to do so, since order or shutdown matters!
		for ( auto& s : mSubsystems ) 
		{ 
			delete s.second;
			s.second = nullptr; 
		}

		mSubsystems.clear( );
	}
	
	//======================================================================================================
			
	SubsystemCatalog* SubsystemCatalog::GetInstance() const
	{
		return mInstance;
	}
	
	//======================================================================================================

	const Subsystem* SubsystemCatalog::Get( const MetaClass* cls ) const
	{
		// Get id of class
		u32 id = cls->GetTypeId( ); 

		if ( Exists( id ) )
		{
			return const_cast< SubsystemCatalog* >( this )->mSubsystems[ id ];
		}

		return nullptr;
	}
	
	//======================================================================================================

	bool SubsystemCatalog::Exists( u32& systemId ) const
	{
		return ( mSubsystems.find( systemId ) != mSubsystems.end( ) );
	}
	
	//====================================================================================================== 
}
