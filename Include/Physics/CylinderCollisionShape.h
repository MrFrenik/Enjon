// @file CylinderCollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_CYLINDERCOLLISIONSHAPE_H
#define ENJON_CYLINDERCOLLISIONSHAPE_H

#include "Physics/CollisionShape.h"
#include "Math/Vec3.h"

namespace Enjon
{ 
	ENJON_CLASS( )
	class CylinderCollisionShape : public CollisionShape
	{
		ENJON_CLASS_BODY( CylinderCollisionShape )

		public:

			/**
			* @brief
			*/
			virtual void ExplicitConstructor( ) override;

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( ) override; 

		private:

			/**
			* @brief
			*/
			virtual void Base( ) override;

		private: 
			ENJON_PROPERTY( )
			Vec3 mHalfExtents = Vec3( 1.0f ); 
	};
}

#endif