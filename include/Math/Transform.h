// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Transform.h

#pragma once
#ifndef ENJON_TRANSFORM_H
#define ENJON_TRANSFORM_H

#include <Defines.h>
#include <System/Types.h>
#include <Math/Common.h>
#include <Math/Quaternion.h>
#include <Math/Mat4.h>
#include <Math/Vec3.h> 
#include <Base/Object.h>

namespace Enjon 
{ 
	enum class TransformSpace
	{
		Local,
		World
	};

	class Transform
	{ 
		public:

			/**
			* @brief Default constructor
			*/
			Transform();

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform(const Vec3& position, const Quaternion& rotation, const Vec3& scale);
			
			/**
			* @brief Copy constructor
			* @param
			* @return
			*/
			Transform(const Transform& t);

			/**
			* @brief 
			* @param
			* @return
			*/
			~Transform();

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform operator*(const Transform& rhs) const;	

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform& operator*=(const Transform& rhs);

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform operator/(Transform& rhs);

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform Inverse();

			/**
			* @brief 
			* @param
			* @return
			*/
			Vec3 GetPosition( ) const;
			
			/**
			* @brief 
			* @param
			* @return
			*/
			Vec3 GetScale( ) const;
			
			/**
			* @brief 
			* @param
			* @return
			*/
			Quaternion GetRotation( ) const;

			/**
			* @brief 
			* @param
			* @return
			*/
			void SetPosition(const Vec3& position);
			
			/**
			* @brief 
			* @param
			* @return
			*/
			void SetScale(const Vec3& scale);

			/**
			* @brief 
			* @param
			* @return
			*/
			void SetScale(const f32& scale);
			
			/**
			* @brief 
			* @param
			* @return
			*/
			void SetRotation(const Quaternion& rotation);

			/**
			* @brief 
			* @param
			* @return
			*/
			void LookAt( const Vec3& target, const Vec3& up = Vec3( 0.0f, 1.0f, 0.0f ) );

		private: 
			Vec3 mPosition;

			Quaternion mRotation;

			Vec3 mScale;
	}; 
}


#endif