// File: Archetype.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Entity/Archetype.h"

namespace Enjon
{ 
	//=======================================================================================

	Result Archetype::SerializeData( ByteBuffer* buffer ) const 
	{
		return Result::INCOMPLETE;
	}

	//=======================================================================================

	Result Archetype::DeserializeData( ByteBuffer* buffer )
	{
		return Result::INCOMPLETE; 
	}

	//=======================================================================================
}


