// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Vec3.h

#pragma once
#ifndef ENJON_VEC3_H
#define ENJON_VEC3_H

#include <iostream>

#include "Math/Vec2.h" 

namespace Enjon {  

	class Quaternion;

	class Vec3
	{ 
		public:

			/**
			* @brief Default constructor
			*/
			Vec3( )
				: x( 0.0f ), y( 0.0f ), z( 0.0f )
			{ 
			}
 
			/**
			* @brief Constructor that takes f32's for x, y, and z
			* @param x - f32 value to be set for x component
			* @param y - f32 value to be set for y component
			* @param z - f32 value to be set for y component
			*/
			Vec3(const f32& _x, const f32& _y, const f32& _z)
				: x(_x), y(_y), z(_z) 
			{
			}
			
			/**
			* @brief Constructor that takes a vec2 and a value for z component
			* @param a - vec2 to be used for x and y components
			* @param val - f32 value to be set for z component
			*/
			Vec3(const Vec2& a, const f32& val)
				: x(a.x), y(a.y), z(val)
			{
			}

			/**
			* @brief Constuctor that takes one f32 and assigns it to both x, y, and z
			* @param val - f32 value to be set
			*/
			Vec3(const f32& val)
				: x(val), y(val), z(val)
			{
			}

			/**
			* @brief Function that returns vec2 of x and y components of this vec3
			* @return vec2 consisting of x and y components
			*/
			Vec2 XY() const;
		
			/**
			* @brief Stream operator for displaying components of vector
			* @param stream - ostream to be used
			* @param vector - vector to be streamed
			*/
			friend std::ostream& operator<<(std::ostream& stream, Vec3& vector);	

			/**
			* @brief Adds components of this vector to another
			* @param other - vector to be added to this 
			* @return this
			*/
			Vec3& Add(const Vec3& other); 

			/**
			* @brief Subtracts components of other vector from this
			* @return this
			*/
			Vec3& Subtract(const Vec3& other);
			
			/**
			* @brief Multiplies components of other vector to this
			* @return this
			*/
			Vec3& Multiply(const Vec3& other);
			
			/**
			* @brief Divides components of this vector by other
			* @return this
			*/
			Vec3& Divide(const Vec3& other); 
			
			/**
			* @brief Scales this vector by a f32 scalar
			* @param other - vector to be used as divisor 
			* @return this
			*/
			Vec3& Scale(const f32& scalar);

			/**
			* @brief Adds one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec3 - The sum of the two vectors as new vector
			*/
			friend Vec3 operator+(Vec3 left, const Vec3& other);

			/**
			* @brief Subtracts one vector from another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec3 - The difference of the two vectors as new vector
			*/
			friend Vec3 operator-(Vec3 left, const Vec3& other);
			
			/**
			* @brief Multiplies one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec3 - The product of the two vectors as new vector
			*/
			friend Vec3 operator*(Vec3 left, const Vec3& other);

			/**
			* @brief Multiplies one vector by scalar 
			* @param left - LH vector in operation
			* @param scale - RH f32 scalar in operation
			* @return Vec3 - New scaled vector
			*/
			friend Vec3 operator*(Vec3 left, const f32& scalar);
			
			
			/**
			* @brief Multiplies one vector by scalar 
			* @param left - LH vector in operation
			* @param scale - RH f32 scalar in operation
			* @return Vec3 - New scaled vector
			*/
			friend Vec3 operator*(const f32& scalar, Vec3 right); 
			
			/**
			* @brief Divides one vector by another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec3 - The quotient of the two vectors as new vector
			*/
			friend Vec3 operator/(Vec3 left, const Vec3& other);
			
			/**
			* @brief Divides one vector by scalar 
			* @param left - LH vector in operation
			* @param scalar - f32 scalar in operation
			* @return Vec3 - New scaled vector
			*/
			friend Vec3 operator/(Vec3 left, const f32& scalar);

			/**
			* @brief Equality operator to check whether two vectors have same components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if same, False if not
			* @return
			*/
			friend b8 operator==(const Vec3& left, const Vec3& right); 
			
			/**
			* @brief Inequality operator to check whether two vectors have different components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if not same, False if same
			*/
			friend b8 operator!=(const Vec3& left, const Vec3& right);
			
			/**
			* @brief Multiplies other vector's components by f32 scalar
			* @param scalar - f32 scalar
			* @return void
			*/
			friend void operator*=(Vec3& left, const f32& scalar);

			/**
			* @brief Multiplies other vector's components by f32 scalar
			* @param scalar - f32 scalar
			* @return void
			*/
			friend void operator*=(const f32& scalar, Vec3& left);

			/**
			* @brief Adds other vector's components onto this vector's components
			* @param other - vector to be added
			* @return this
			*/
			Vec3& operator+=(const Vec3& other);

			/**
			* @brief Subtracts other vector's components from this vector's components
			* @param other - vector to be subtracted
			* @return this
			*/
			Vec3& operator-=(const Vec3& other);
			
			/**
			* @brief Multiplies other vector's components by this vector's components
			* @param other - vector to be multiplied
			* @return this
			*/
			Vec3& operator*=(const Vec3& other); 

			/**
			* @brief Divides other vector's components into this vector's components
			* @param other - vector to be divided by
			* @return this
			*/
			Vec3& operator/=(const Vec3& other);

			/**
			* @brief Negates vector components
			* @return New negated vector
			*/ 
			Vec3 operator-();
	
			/**
			* @brief
			* @param
			* @return
			*/
			Vec3 operator*( const Quaternion& V ) const;

			/**
			* @brief Performs dot product with other vector and returns the calculated value
			* @param other - vector to perform dot product with
			* @return f32 - value of dot product
			*/
			f32 Dot(const Vec3& other) const;

			/**
			* @brief Performs cross product with other vector and returns the calculated value
			* @param other - vector to perform cross product with
			* @return Vec3 - vector result of cross product
			*/
			Vec3 Cross(const Vec3& other) const;
			
			/**
			* @brief Performs normalization on this vector and returns it
			* @return Vec3 - normalized vector
			*/
			Vec3 Normalize() const;
			
			/** 
			* @brief Calculates magnitude of this vector
			* @return f32 - magnitude of vector
			*/	
			f32 Vec3::Length() const;

			/** 
			* @brief Calculate squared magnitude of vector
			* @return f32 - squared magnitude of vector
			*/	
			f32 Vec3::Length2() const;

			/** 
			* @brief Calculates distance between two vectors
			* @param other - RH vector used in distance calculation
			* @return f32 - distance between two vectors
			*/	
			f32 Distance(const Vec3& other) const;

			/**
			* @brief Static function which returns basis vector along X axis
			* @return Vec3 - Basis vector along X axis
			*/
			static Vec3 XAxis();
			
			/**
			* @brief Static function which returns basis vector along Y axis
			* @return Vec3 - Basis vector along Y axis
			*/
			static Vec3 YAxis();
			
			/**
			* @brief Static function which returns basis vector along Z axis
			* @return Vec3 - Basis vector along Z axis
			*/
			static Vec3 ZAxis();
			
			/**
			* @brief Static function that calculates squared distance from one vector to another
			* @param a - LH vector used in distance calculation
			* @param b - RH vector used in distance calculation
			* @return f32 - squared distance between two vectors
			*/
			static f32 Distance(const Vec3& a, const Vec3& b); 

			/** 
			* @brief Static function that accepts a vector and returns a new 
			*		vector that is normalized from the one passed in
			* @param vec - the vector to be used for the normalization
			* @return Vec3 - new normalized vector
			*/	
			static Vec3 Normalize(const Vec3& vec);

			/**
			* @brief Static function that calculates squared distance from one vector to another
			* @param a - LH vector used in distance calculation
			* @param b - RH vector used in distance calculation
			* @return f32 - squared distance between two vectors
			*/
			static f32 DistanceSquared(const Vec3& a, const Vec3& b); 
		
		public:
			f32 x; 
			f32 y;
			f32 z;
	}; 
	
	class iVec3
	{
		public:

			/**
			* @brief Default constructor
			*/
			iVec3() = default; 

			/**
			* @brief Constructor that takes s32's for x and y 		
			* @param x - s32 value to be set for x component
			* @param y - s32 value to be set for y component
			*/
			iVec3(const s32& _x, const s32& _y)
				: x(_x), y(_y)
			{
			} 

			/**
			* @brief Constuctor that takes one s32 and assigns it to both x and y
			* @param val - s32 value to be set
			*/
			iVec3(const s32& val)
				: x(val), y(val)
			{
			}
			
			/**
			* @brief Default Destructor
			*/
			~iVec3() {}
			
			/**
			* @brief Stream operator for displaying components of vector
			* @param stream - ostream to be used
			* @param vector - vector to be streamed
			*/
			friend std::ostream& operator<<(std::ostream& stream, iVec3& vector);	

			/**
			* @brief Adds components of this vector to another
			* @param other - vector to be added to this 
			* @return this
			*/
			iVec3& Add(const iVec3& other);
			
			/**
			* @brief Subtracts components of other vector from this
			* @return this
			*/
			iVec3& Subtract(const iVec3& other);
			
			/**
			* @brief Multiplies components of other vector to this
			* @return this
			*/
			iVec3& Multiply(const iVec3& other);
			
			/**
			* @brief Divides components of this vector by other
			* @return this
			*/
			iVec3& Divide(const iVec3& other); 
			
			/**
			* @brief Scales this vector by a f32 scalar
			* @param other - vector to be used as divisor 
			* @return this
			*/
			iVec3& Scale(const s32& scalar);

			/**
			* @brief Adds one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec3 - The sum of the two vectors as new vector
			*/
			friend iVec3 operator+(iVec3 left, const iVec3& other);
			
			/**
			* @brief Subtracts one vector from another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec3 - The difference of the two vectors as new vector
			*/
			friend iVec3 operator-(iVec3 left, const iVec3& other); 
			
			/**
			* @brief Multiplies one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec3 - The product of the two vectors as new vector
			*/
			friend iVec3 operator*(iVec3 left, const iVec3& other);
			
			/**
			* @brief Divides one vector by another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec3 - The quotient of the two vectors as new vector
			*/
			friend iVec3 operator/(iVec3 left, const iVec3& other);
			
			/**
			* @brief Equality operator to check whether two vectors have same components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if same, False if not
			* @return
			*/
			friend b8 operator==(iVec3 left, const iVec3& other);
			
			/**
			* @brief Inequality operator to check whether two vectors have different components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if not same, False if same
			*/
			friend b8 operator!=(iVec3 left, const iVec3& other);
			
			/**
			* @brief Adds other vector's components onto this vector's components
			* @param other - vector to be added
			* @return this
			*/
			iVec3& operator+=(const iVec3& other);
			
			/**
			* @brief Subtracts other vector's components from this vector's components
			* @param other - vector to be subtracted
			* @return this
			*/
			iVec3& operator-=(const iVec3& other);
			
			/**
			* @brief Multiplies other vector's components by this vector's components
			* @param other - vector to be multiplied
			* @return this
			*/
			iVec3& operator*=(const iVec3& other);
			
			/**
			* @brief Divides other vector's components into this vector's components
			* @param other - vector to be divided by
			* @return this
			*/
			iVec3& operator/=(const iVec3& other); 
			
			/**
			* @brief Multiplies vector by f32 scalar quantity
			* @param scalar - quantity to multiply by
			* @return this
			*/
			iVec3& operator*=(const s32& scalar);
			
		public:
			s32 x; 
			s32 y; 
			s32 z;
	};
}

typedef Enjon::Vec3 v3;

#endif