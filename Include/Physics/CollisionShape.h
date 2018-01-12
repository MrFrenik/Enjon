// @file CollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_COLLISIONSHAPE_H
#define ENJON_COLLISIONSHAPE_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Physics/PhysicsDefines.h" 
#include "Math/Vec3.h"

namespace Enjon
{
	class RigidBody;
	class PhysicsSubsystem;

	ENJON_ENUM()
	enum class CollisionShapeType
	{
		Empty,
		Box,
		Sphere,
		Cylinder,
		Cone,
		Capsule
	};

	ENJON_CLASS( )
	class CollisionShape : public Object 
	{
		friend RigidBody;
		friend PhysicsSubsystem;

		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief
			*/
			CollisionShape( );

			/**
			* @brief
			*/
			~CollisionShape( );

		protected:

			/**
			* @brief
			*/
			BV3 CalculateLocalInertia( const f32& mass );

			/**
			* @brief
			*/
			BulletCollisionShape* GetRawShape( ) const;

			/**
			* @brief
			*/
			void SetLocalScaling( const Vec3& scale );

			/**
			* @brief
			*/
			Vec3 GetLocalScaling( );

			/**
			* @brief
			*/
			CollisionShapeType GetCollisionShapeType( ) const;

		private:

			/**
			* @brief Make this class completely abstract
			*/
			virtual void Base( ) = 0;

 
		protected: 
			ENJON_PROPERTY( )
			Vec3 mLocalScaling = Vec3( 1.0f );

			ENJON_PROPERTY( )
			CollisionShapeType mShapeType = CollisionShapeType::Empty;

		protected: 
			BulletCollisionShape* mShape = nullptr;
	}; 
}

#endif
