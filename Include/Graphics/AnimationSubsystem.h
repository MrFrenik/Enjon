// @file AnimationSubsystem.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_ANIMATION_SUBSYSTEM_H
#define ENJON_ANIMATION_SUBSYSTEM_H

#include "System/Types.h"
#include "Subsystem.h"

namespace Enjon
{
	ENJON_CLASS( )
	class AnimationSubsystem : public Subsystem 
	{
		ENJON_CLASS_BODY( AnimationSubsystem )

		public:

			/**
			*@brief
			*/
			virtual Result Initialize() override;

			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) override;

			/**
			*@brief
			*/
			virtual Result Shutdown() override;

		protected: 
	};
}

#endif
