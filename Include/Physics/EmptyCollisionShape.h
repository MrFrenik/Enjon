// @file EmptyCollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EMPTYCOLLISIONSHAPE_H
#define ENJON_EMPTYCOLLISIONSHAPE_H

#include "Physics/CollisionShape.h"

namespace Enjon
{ 
	ENJON_CLASS( Construct )
	class EmptyCollisionShape : public CollisionShape
	{
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief
			*/
			EmptyCollisionShape( );

			/**
			* @brief
			*/
			EmptyCollisionShape( RigidBody* body );

			/**
			* @brief
			*/
			~EmptyCollisionShape( );

		private:

			/**
			* @brief
			*/
			virtual void Base( ) override;
	};
}

#endif
