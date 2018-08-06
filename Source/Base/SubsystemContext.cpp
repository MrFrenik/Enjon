// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file SubsystemContext.cpp

#include "Base/SubsystemContext.h"
#include "Base/World.h"

namespace Enjon
{
	//=====================================================================================

	SubsystemContext::SubsystemContext( World* world )
		: mWorld( world )
	{
	}

	//=====================================================================================

	void SubsystemContext::SetUpdates( bool updates )
	{
		mUpdates = updates;
	}

	//=====================================================================================

	bool SubsystemContext::GetUpdates( ) const
	{
		return mUpdates;
	}

	//=====================================================================================
}

