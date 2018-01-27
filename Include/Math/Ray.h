// @file Ray.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_RAY_H
#define ENJON_RAY_H

#include "Math/Vec3.h"

namespace Enjon
{
	class Ray
	{
		public:
			
			/**
			* @brief Constructor
			*/
			Ray( );

			/**
			* @brief Constructor
			*/
			Ray( const Vec3& point, const Vec3& direction );

			/**
			* @brief Destructor
			*/
			~Ray( );

		public:

			Vec3 mPoint;
			Vec3 mDirection; 
	};
}

#endif
