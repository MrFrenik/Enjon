// @file SphereCollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SPHERECOLLISIONSHAPE_H
#define ENJON_SPHERECOLLISIONSHAPE_H

#include "Physics/CollisionShape.h"
#include "Math/Vec3.h"

namespace Enjon
{ 
	ENJON_CLASS( Construct )
	class SphereCollisionShape : public CollisionShape
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief
			*/
			SphereCollisionShape( );

			/**
			* @brief
			*/
			~SphereCollisionShape( );

		private:

			/**
			* @brief
			*/
			virtual void Base( ) override;

		private:

			ENJON_PROPERTY( )
			f32 mRadius = 1.0f;
	};
}

#endif
