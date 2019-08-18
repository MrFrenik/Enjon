// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Quaternion.h

#pragma once
#ifndef ENJON_QUATERNION_H
#define ENJON_QUATERNION_H

#include <ostream>

#include <Defines.h>
#include <System/Types.h>
#include <Math/Vec3.h>
#include <Math/Common.h>
#include <Math/Mat4.h>

namespace Enjon 
{ 
	class Quaternion
	{
		public: 

			Quaternion() 
				: x(0.0f), y(0.0f), z(0.0f), w(1.0f) 
			{}

			Quaternion(const f32& _x, const f32& _y, const f32& _z, const f32& _w) 
				: x(_x), y(_y), z(_z), w(_w) 
			{
			}

			Quaternion(const Vec3& vec, f32 _w)
				: x(vec.x), y(vec.y), z(vec.z), w(_w)
			{}

			Quaternion::Quaternion(const Quaternion& q)
				: x(q.x), y(q.y), z(q.z), w(q.w) 
			{} 

			~Quaternion( )
			{ 
			}

			// Return vec3 of x,y,z components of quaternion
			Vec3 XYZ();

			// Negative unary operator overload
			Quaternion operator-() const;

			// Addition operator overload
			Quaternion operator+(const Quaternion& Other) const;

			// Subtraction operator overload
			Quaternion operator-(const Quaternion& Other) const;

			// Multiplication operator overload
			Quaternion operator*(const Quaternion& Other) const;

			// Multiplication operator with scalar
			Quaternion operator*(const f32& V) const;

			// Division operator overload with f32
			Quaternion operator/(const f32& V) const; 

			Quaternion& operator*=( const Quaternion& other );
		
			Vec3 operator*(const Vec3& V) const;

			// Equality operator
			bool operator==(const Quaternion& Other) const;

			// Inequality operator
			bool operator!=(const Quaternion& Other) const;

			// Operator stream overload
			friend std::ostream& operator<<(std::ostream& stream, const Quaternion& Q); 

			// Gets length of quaternion
			f32 Length();

			Vec3 Rotate( const Vec3& v ) const;

			// Normalizes length of quaternion to 1
			Quaternion Normalize();

			// Returns conjugate of quaternion
			Quaternion Conjugate() const;

			// Returns dot product with another quaternion
			f32 Dot( const Quaternion& Q ) const;

			// Returns cross product with another quaternion
			Quaternion Cross(const Quaternion& Q) const;

			// Returns inverse of quaternion
			Quaternion Inverse(Quaternion& Q);
			
			// Returns inverse of quaternion
			Quaternion Inverse();

			// Get angle from quaternion in radians
			f32 Angle() const;

			// Get axis from quaternion
			Vec3 Axis() const;

			// Get Euler roll from quaternion
			f32 Roll();

			// Get Euler pitch from quaternion
			f32 Pitch();

			// Get Euler yaw from quaternion
			f32 Yaw();

			// Get Euler Angles from quaternion
			Vec3 EulerAngles(); 

			/**
			* @brief
			*/
			Quaternion NegativeAngleAxis( ) const;

			/*
			* @brief
			* @note implementation from: https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp
			*/
			static Quaternion RotationBetweenVectors( const Vec3& start, const Vec3& destination ); 

			/*
			* @brief
			*/
			static Quaternion Slerp( const Quaternion& a, const Quaternion& b, const f32& t ); 

			/*
			* @brief
			*/
			static Quaternion FromEulerAngles( const f32& yawDeg, const f32& pitchDeg, const f32& rollDeg );

			/*
			* @brief
			*/
			static inline Quaternion FromEulerAngles( const Vec3& eulerAngles )
			{
				return Quaternion::FromEulerAngles( eulerAngles.x, eulerAngles.y, eulerAngles.z );
			}
			
			/*
			* @brief
			*/
			static inline Quaternion AngleAxis(const f32& Angle, const Vec3& Axis)
			{
				Quaternion Q;

				// Normalize axis first	
				Vec3 A = Vec3::Normalize(Axis);

				// Get scalar
				const f32 halfAngle = 0.5f * Angle;
				const f32 S = std::sinf(halfAngle); 

				// Set Q
				Q.x = A.x * S;
				Q.y = A.y * S;
				Q.z = A.z * S;
				Q.w = std::cosf(halfAngle);

				return Q;
			}

			// Member variables
			f32 x, y, z, w;
	}; 

	/*
	* @brief Convert this quaternion into equivalent 4X4 rotation matrix
	* @note: From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm 
	*/
	static inline Enjon::Mat4x4 QuaternionToMat4x4(Quaternion B)
	{
		Mat4x4 Mat = Mat4x4::Identity();
		Quaternion Q = B.Normalize();

		f32 xx = Q.x*Q.x;	
		f32 yy = Q.y*Q.y;	
		f32 zz = Q.z*Q.z;	
		f32 xy = Q.x*Q.y;
		f32 xz = Q.x*Q.z;
		f32 yz = Q.y*Q.z;
		f32 wx = Q.w*Q.x;
		f32 wy = Q.w*Q.y;
		f32 wz = Q.w*Q.z;

		Mat.Elements[0][0] = 1.0f - 2.0f * (yy + zz);
		Mat.Elements[1][0] = 2.0f * (xy - wz);
		Mat.Elements[2][0] = 2.0f * (xz + wy);

		Mat.Elements[0][1] = 2.0f * (xy + wz);
		Mat.Elements[1][1] = 1.0f - 2.0f * (xx + zz);
		Mat.Elements[2][1] = 2.0f * (yz - wx);

		Mat.Elements[0][2] = 2.0f * (xz - wy);
		Mat.Elements[1][2] = 2.0f * (yz + wx);
		Mat.Elements[2][2] = 1.0f - 2.0f * (xx + yy);

		return Mat;
	}

	// NOTE(John): Assumes matrix is only rotational matrix and has no skew applied
	static inline Quaternion Mat4x4ToQuaternion(const Mat4x4& Mat)
	{
		Quaternion Q;
		auto& E = Mat.elements;

		f32 FourXSquaredMinusOne 	= E[0 * 4 + 0] - E[1 * 4 + 1] - E[2 * 4 + 2];
		f32 FourYSquaredMinusOne 	= E[1 * 4 + 1] - E[0 * 4 + 0] - E[2 * 4 + 2];
		f32 FourZSquaredMinuesOne 	= E[2 * 4 + 2] - E[0 * 4 + 0] - E[1 * 4 + 1];
		f32 FourWSquaredMinusOne 	= E[0 * 4 + 0] + E[1 * 4 + 1] + E[2 * 4 + 2];

		int32 BiggestIndex = 0;
		f32 FourBiggestSquaredMinusOne = FourWSquaredMinusOne;

		if (FourXSquaredMinusOne > FourBiggestSquaredMinusOne)
		{
			FourBiggestSquaredMinusOne 	= FourXSquaredMinusOne;
			BiggestIndex 				= 1;
		}
		if (FourYSquaredMinusOne > FourBiggestSquaredMinusOne)
		{
			FourBiggestSquaredMinusOne 	= FourYSquaredMinusOne;
			BiggestIndex 				= 2;
		}
		if (FourZSquaredMinuesOne > FourBiggestSquaredMinusOne)
		{
			FourBiggestSquaredMinusOne 	= FourZSquaredMinuesOne;
			BiggestIndex 				= 3;
		}


		f32 BiggestVal 	= std::sqrt(FourBiggestSquaredMinusOne + 1.0f) * 0.5f;
		f32 Mult 		= 0.25f / BiggestVal;

		switch(BiggestIndex)
		{
			case 0:
			{
				Q.w = BiggestVal;
				Q.x = (E[2 * 4 + 1] - E[1 * 4 + 2]) * Mult;
				Q.y = (E[0 * 4 + 2] - E[2 * 4 + 0]) * Mult;
				Q.z = (E[1 * 4 + 0] - E[0 * 4 + 1]) * Mult;
			}
			break;
			case 1:
			{
				Q.w = (E[2 * 4 + 1] - E[1 * 4 + 2]) * Mult;
				Q.x = BiggestVal;
				Q.y = (E[1 * 4 + 0] + E[0 * 4 + 1]) * Mult;
				Q.z = (E[0 * 4 + 2] + E[2 * 4 + 0]) * Mult;
			}
			break;
			case 2:
			{
				Q.w = (E[0 * 4 + 2] - E[2 * 4 + 0]) * Mult;
				Q.x = (E[1 * 4 + 0] + E[0 * 4 + 1]) * Mult;
				Q.y = BiggestVal;
				Q.z = (E[2 * 4 + 1] + E[1 * 4 + 2]) * Mult;
			}
			break;
			case 3:
			{
				Q.w = (E[1 * 4 + 0] - E[0 * 4 + 1]) * Mult;
				Q.x = (E[0 * 4 + 2] + E[2 * 4 + 0]) * Mult;
				Q.y = (E[2 * 4 + 1] + E[1 * 4 + 2]) * Mult;
				Q.z = BiggestVal;
			}
			break;
			default:
			{

			}
			break; 
		}

		return Q;
	} 
}

typedef Enjon::Quaternion quat;

#endif