// @file SkeletalAnimation.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SKELETAL_ANIMATION_H
#define ENJON_SKELETAL_ANIMATION_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Math/Maths.h"

namespace Enjon
{
	class Animation;

	class KeyFrame
	{ 
		public:

			/*
			* @brief
			*/
			KeyFrame( ) = default;

			/*
			* @brief
			*/
			~KeyFrame( ) = default;

		protected:

		private: 
			Vector< Mat4x4 > mJointTransforms;
			f32 mTimeStamp;
	};

	class Animation
	{ 
		public:

			/*
			* @brief
			*/
			Animation( ) = default;

			/*
			* @brief
			*/
			~Animation( ) = default;

		protected:

		private:
			Vector< KeyFrame > mKeyFrames;
	};
}

#endif
