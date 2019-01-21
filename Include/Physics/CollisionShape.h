// @file CollisionShape.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_COLLISIONSHAPE_H
#define ENJON_COLLISIONSHAPE_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Physics/PhysicsDefines.h" 
#include "Physics/RigidBody.h"
#include "Math/Vec3.h"

namespace Enjon
{
	class RigidBody;
	class PhysicsSubsystem;
	class RigidBodyComponent;

	ENJON_ENUM( )
	enum class CollisionShapeType
	{
		Empty,
		Box,
		Sphere,
		Cylinder,
		Cone,
		Capsule
	};

	ENJON_CLASS( Abstract )
	class CollisionShape : public Object 
	{
		friend RigidBody;
		friend PhysicsSubsystem;
		friend RigidBodyComponent;

		ENJON_CLASS_BODY( CollisionShape )

		public: 

			/**
			* @brief
			*/
			CollisionShape( const ResourceHandle< RigidBody >& body );

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( ) override;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			void SetOffset( const Vec3& offset ); 

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			Vec3 GetOffset( ); 

		protected:

			/**
			* @brief
			*/
			void SetBody( const ResourceHandle< RigidBody >& body );

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

			/**
			* @brief
			*/
			void DeleteShape( );

		private:

			/**
			* @brief Make this class completely abstract
			*/
			virtual void Base( ) = 0;

 
		protected: 
			ENJON_PROPERTY( )
			Vec3 mLocalScaling = Vec3( 1.0f );

			ENJON_PROPERTY( UIMin = -5.0f, UIMax = 5.0f, Delegates[ Mutator = SetOffset ] )
			Vec3 mOffset = Vec3( 0.0f );

			ENJON_PROPERTY( )
			CollisionShapeType mShapeType = CollisionShapeType::Empty;

		protected: 
			BulletCollisionShape* mShape = nullptr; 
			ResourceHandle< RigidBody > mBody;
	}; 
}

#endif
