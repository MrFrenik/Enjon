// @file SkeletalMeshRenderable.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SKELETAL_MESH_RENDERABLE_H
#define ENJON_SKELETAL_MESH_RENDERABLE_H

#include "System/Types.h"
#include "Graphics/Renderable.h"
#include "Graphics/SkeletalMesh.h"

namespace Enjon
{
	class AnimationSubsystem;
	class SkeletalAnimationComponent;
	class SkeletalMeshComponent;

	ENJON_CLASS( )
	class SkeletalMeshRenderable : public Renderable
	{
		friend AnimationSubsystem;
		friend SkeletalAnimationComponent;
		friend SkeletalMeshComponent;

		ENJON_CLASS_BODY( SkeletalMeshRenderable )

		public: 

			/**
			* @brief
			*/
			virtual void SetMesh( const Mesh* mesh ) override;

			/**
			* @brief
			*/
			void SetMesh( const AssetHandle< SkeletalMesh >& mesh ); 

			/**
			* @brief
			*/
			virtual const Mesh* GetMesh() const override; 

			/**
			* @brief
			*/
			AssetHandle< Skeleton > GetSkeleton( ) const;

			/**
			* @brief
			*/
			const Vector< Mat4x4 >& GetJointTransforms( ) const; 

		protected:

			ENJON_PROPERTY( Editable, Delegates[ Mutator = SetMesh ] )
			AssetHandle< SkeletalMesh > mMesh;

			Vector< Mat4x4 > mFinalJointTransforms; 
	};
}

#endif
