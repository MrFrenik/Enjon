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
	ENJON_ENUM( )
	enum class TransformSpace
	{
		Local,
		World
	};

	/*
	* @brief VQS transform
	*/
	ENJON_CLASS( )
	class Transform : public Object
	{ 
		ENJON_CLASS_BODY( Transform )

		public:

			/**
			* @brief Default constructor
			*/
			virtual void ExplicitConstructor( ) override;

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform( const Vec3& position, const Quaternion& rotation, const Vec3& scale );
			
			/**
			* @brief Copy constructor
			* @param
			* @return
			*/
			Transform( const Transform& t ); 

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform operator*( const Transform& rhs ) const;

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform& operator*=( const Transform& rhs );

			/**
			* @brief 
			* @param
			* @return
			*/
			Transform operator/( const Transform& rhs ) const;

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
			Vec3 GetEulerAngles( ) const;

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
			* @brief Accept euler angles ( in degrees )
			* @param
			* @return
			*/
			ENJON_FUNCTION( )
			void SetEulerRotation(const Vec3& eulerAngles);

			/**
			* @brief 
			* @param
			* @return
			*/
			void LookAt( const Vec3& target, const Vec3& up = Vec3( 0.0f, 1.0f, 0.0f ) );

			/**
			* @brief 
			* @param
			* @return
			*/
			Mat4x4 ToMat4x4( ) const; 

		private: 

			ENJON_PROPERTY( )
			Vec3 mPosition;

			ENJON_PROPERTY( HideInEditor, Delegates[ Mutator = SetRotation ] )
			Quaternion mRotation;

			ENJON_PROPERTY( )
			Vec3 mScale;

			ENJON_PROPERTY( Delegates[ Mutator = SetEulerRotation ] )
			Vec3 mEulerAngles;
	}; 
}


#endif