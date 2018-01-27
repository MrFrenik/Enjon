// @file Ray.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Math/Ray.h"

namespace Enjon
{
	//=============================================================================

	Ray::Ray( )
		: mPoint( Vec3( 0.0f ) ), mDirection( Vec3( 0.0f, 0.0f, -1.0f ) )
	{ 
	}

	//=============================================================================

	Ray::Ray( const Vec3& point, const Vec3& direction )
		: mPoint( point ), mDirection( direction )
	{ 
	}

	//=============================================================================

	Ray::~Ray( )
	{ 
	}

	//=============================================================================

}
