// @file Plane.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_PLANE_H
#define ENJON_PLANE_H

#include "Math/Vec3.h"

namespace Enjon
{
	struct LineIntersectionResult 
	{
		u32 mHit;
		Vec3 mHitPosition;
	};

	class Plane
	{
		public: 

			/**
			* @brief Constructor
			*/
			Plane( );

			/**
			* @brief Constructor
			*/
			Plane( const Plane& other );

			/**
			* @brief Constructor
			*/
			Plane( const f32& a, const f32& b, const f32& c, const f32& d );

			/**
			* @brief Constructor
			*/
			Plane( const Vec3& normal, const Vec3& point );

			/**
			* @brief Constructor
			*/
			Plane( const Vec3& p0, const Vec3& p1, const Vec3& p2 ); 

			/**
			* @brief Destructor
			*/
			~Plane( );

			/**
			* @brief Destructor
			*/
			LineIntersectionResult GetLineIntersection( const Vec3& lineStart, const Vec3& lineEnd );

		private:

			f32 a;
			f32 b;
			f32 c;
			f32 d;
	};
}

#endif
