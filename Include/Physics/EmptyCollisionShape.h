// @file EmptyCollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EMPTYCOLLISIONSHAPE_H
#define ENJON_EMPTYCOLLISIONSHAPE_H

#include "Physics/CollisionShape.h"

namespace Enjon
{ 
	ENJON_CLASS( )
	class EmptyCollisionShape : public CollisionShape
	{
		ENJON_CLASS_BODY( EmptyCollisionShape )

		public:

			/**
			* @brief
			*/
			virtual void ExplicitConstructor( ) override;

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( ) override;

			/**
			* @brief
			*/
			EmptyCollisionShape( const ResourceHandle< RigidBody >& body );


		private:

			/**
			* @brief
			*/
			virtual void Base( ) override;
	};
}

#endif
