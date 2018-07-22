// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file World.cpp

#include "Base/World.h"
#include "Base/SubsystemContext.h"
#include "Entity/EntityManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{ 
	//===============================================================

	World::~World( )
	{
		// Remove world from entity manager
		EngineSubsystem( EntityManager )->RemoveWorld( this );

		// Release all contexts
		for ( auto& c : mContextMap )
		{
			delete ( c.second );
			c.second = nullptr;
		} 
	}

	//===============================================================

}

