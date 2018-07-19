// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file World.cpp

#include "Base/World.h"
#include "Base/SubsystemContext.h"

namespace Enjon
{ 
	//===============================================================

	World::~World( )
	{
		// Release all contexts
		for ( auto& c : mContextMap )
		{
			delete ( c.second );
			c.second = nullptr;
		}
	}

	//===============================================================

}

