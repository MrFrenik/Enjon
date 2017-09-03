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

namespace Enjon 
{ 
	enum class TransformSpace
	{
		LOCAL,
		WORLD
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
			Vec3 GetPosition() const { return Position; }
			
			/**
			* @brief 
			* @param
			* @return
			*/
			Vec3 GetScale() const { return Scale; }
			
			/**
			* @brief 
			* @param
			* @return
			*/
			Quaternion GetRotation() const { return Rotation; }

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

		public: 
			Vec3 Position;

			Quaternion Rotation;

			Vec3 Scale;
	}; 
}


#endif