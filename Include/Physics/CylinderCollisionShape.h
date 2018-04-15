// @file CylinderCollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_CYLINDERCOLLISIONSHAPE_H
#define ENJON_CYLINDERCOLLISIONSHAPE_H

#include "Physics/CollisionShape.h"
#include "Math/Vec3.h"

namespace Enjon
{ 
	ENJON_CLASS( Construct )
	class CylinderCollisionShape : public CollisionShape
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief
			*/
			CylinderCollisionShape( );

			/**
			* @brief
			*/
			CylinderCollisionShape( RigidBody* body );

			/**
			* @brief
			*/
			~CylinderCollisionShape( );

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