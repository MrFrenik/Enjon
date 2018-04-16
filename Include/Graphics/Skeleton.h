// @file Skeleton.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SKELETON_H
#define ENJON_SKELETON_H

#include "System/Types.h"
#include "Defines.h"
#include "Math/Maths.h"
#include "Asset/Asset.h" 

namespace Enjon
{
	class Skeleton;

	class Bone
	{ 
		friend Skeleton;

		public: 

			/*
			* @brief Constructor
			*/
			Bone( );

			/*
			* @brief Constructor
			*/
			~Bone( );

		protected:
			Vector<Bone*>	mChildren;
			u32				mID;
			Transform		mAnimatedTransform;
			Transform		mLocalBindTransform; 
			Transform		mInverseBindTrasform;
			String			mName;
	};

	class Skeleton
	{ 
		public:

			/*
			* @brief
			*/
			Skeleton( );

			/*
			* @brief
			*/
			~Skeleton( ); 

		private: 
			Bone*						mRoot = nullptr;
			Vector< Bone >				mBones;
			HashMap< String, Bone* >	mBoneNameLookup;
	};
}

#endif
