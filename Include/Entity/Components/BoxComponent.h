// @file BoxComponent.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_BOX_COMPONENT_H
#define ENJON_BOX_COMPONENT_H

#include "System/Types.h"
#include "Entity/Components/PhysicsComponent.h"

namespace Enjon
{ 
	ENJON_CLASS( Construct )
	class BoxComponent : public PhysicsComponent
	{
		ENJON_CLASS_BODY( )
		ENJON_COMPONENT( BoxComponent )

		public:

			/*
			* @brief
			*/
			BoxComponent( );

			/*
			* @brief
			*/
			~BoxComponent( );

		private:

			ENJON_PROPERTY( )
			Vec3 mHalfExtents = Vec3( 1.0f, 1.0f, 1.0f );
	};
}

#endif
