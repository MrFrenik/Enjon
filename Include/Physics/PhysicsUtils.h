// @file PhysicsUtils.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#ifndef ENJON_PHYSICS_UTILS_H
#define ENJON_PHYSICS_UTILS_H

#include "Physics/PhysicsDefines.h" 
 
namespace Enjon
{ 
	class Vec3;

	namespace PhysicsUtils
	{
		/**
		* @brief
		*/
		 BV3 Vec3ToBV3( const Vec3& vec ); 

		/**
		* @brief
		*/
		 Vec3 BV3ToVec3( const BV3& vec ); 
	};
}

#endif

