// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// @file: SubsystemCatalog.cpp

#include "SubsystemCatalog.h"

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
		for ( auto& s : mSubsystems )
		{
			s.second->Shutdown( );
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
