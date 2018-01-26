// @file Plane.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Math/Plane.h"

namespace Enjon
{
	//==============================================================================

	Plane::Plane( )
		: a( 0.0f ), b( 0.0f ), c( 0.0f ), d( 0.0f )
	{
	}

	//==============================================================================

	Plane::Plane( const Plane& other )
		: a( other.a ), b( other.b ), c( other.c ), d( other.d )
	{ 
	}

	//==============================================================================

	Plane::Plane( const f32& _a, const f32& _b, const f32& _c, const f32& _d )
		: a( _a ), b( _b ), c( _c ), d( _d )
	{ 
	}

	//==============================================================================

	Plane::Plane( const Vec3& normal, const Vec3& point )
	{
		Vec3 normalizedNormal = normal.Normalize( );
		this->a = normalizedNormal.x;
		this->b = normalizedNormal.y;
		this->c = normalizedNormal.z;
		this->d = -normalizedNormal.Dot( point );
	} 

	//==============================================================================

	Plane::Plane( const Vec3& p0, const Vec3& p1, const Vec3& p2 )
	{ 
		Vec3 normal = Vec3::Cross( p1 - p0, p2 - p0 );
		*this = Plane( normal, p0 );
	} 

	//==============================================================================

	Plane::~Plane( )
	{ 
	}

	//============================================================================== 

	LineIntersectionResult Plane::GetLineIntersection( const Vec3& lineStart, const Vec3& lineEnd )
	{
		LineIntersectionResult result;

		Vec3 dir = ( lineEnd - lineStart ).Normalize();
		f32 denom = this->a * dir.x + this->b * dir.y + this->c * dir.z;
		if ( denom == 0.0f )
		{
			result.mHit = false;
		}
		else
		{
			result.mHit = true;
			f32 t = -( this->a * lineStart.x + this->b * lineStart.y + this->c * lineStart.z + d ) / denom;

			result.mHitPosition = ( lineStart + t * ( dir ) );
		}

		return result;
	}

	//============================================================================== 
}
