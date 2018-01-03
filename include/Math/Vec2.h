// @file Vec2.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_VEC2_H
#define ENJON_VEC2_H

#include <iostream>

#include "System/Types.h"

namespace Enjon 
{ 
	class Vec2
	{ 
		public: 

			/**
			* @brief Default constructor
			*/
			Vec2( )
				: x( 0.0f ), y( 0.0f )
			{ 
			}

			/**
			* @brief Constructor that takes f32's for x and y 		
			* @param x - f32 value to be set for x component
			* @param y - f32 value to be set for y component
			*/
			Vec2(const f32& _x, const f32& _y)
				: x(_x), y(_y)
			{ 
			} 
			
			/**
			* @brief Constuctor that takes one f32 and assigns it to both x and y
			* @param val - f32 value to be set
			*/
			Vec2(const f32& val)
				: x(val), y(val)
			{
			}

			/**
			* @brief Default Destructor
			*/
			~Vec2() {}
			
			/**
			* @brief Stream operator for displaying components of vector
			* @param stream - ostream to be used
			* @param vector - vector to be streamed
			*/
			friend std::ostream& operator<<(std::ostream& stream, Vec2& vector);	

			/**
			* @brief Adds components of this vector to another
			* @param other - vector to be added to this 
			* @return this
			*/
			Vec2& Add(const Vec2& other);
			
			/**
			* @brief Subtracts components of other vector from this
			* @return this
			*/
			Vec2& Subtract(const Vec2& other);
			
			/**
			* @brief Multiplies components of other vector to this
			* @return this
			*/
			Vec2& Multiply(const Vec2& other);
			
			/**
			* @brief Divides components of this vector by other
			* @return this
			*/
			Vec2& Divide(const Vec2& other); 
			
			/**
			* @brief Scales this vector by a f32 scalar
			* @param other - vector to be used as divisor 
			* @return this
			*/
			Vec2& Scale(const f32& scalar);

			/**
			* @brief Adds one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec2 - The sum of the two vectors as new vector
			*/
			friend Vec2 operator+(Vec2 left, const Vec2& other);
			
			/**
			* @brief Subtracts one vector from another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec2 - The difference of the two vectors as new vector
			*/
			friend Vec2 operator-(Vec2 left, const Vec2& other); 
			
			/**
			* @brief Multiplies one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec2 - The product of the two vectors as new vector
			*/
			friend Vec2 operator*(Vec2 left, const Vec2& other);
			
			/**
			* @brief Divides one vector by another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return Vec2 - The quotient of the two vectors as new vector
			*/
			friend Vec2 operator/(Vec2 left, const Vec2& other);
			
			/**
			* @brief Equality operator to check whether two vectors have same components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if same, False if not
			* @return
			*/
			friend b8 operator==(Vec2 left, const Vec2& other);
			
			/**
			* @brief Inequality operator to check whether two vectors have different components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if not same, False if same
			*/
			friend b8 operator!=(Vec2 left, const Vec2& other);
			
			/**
			* @brief Adds other vector's components onto this vector's components
			* @param other - vector to be added
			* @return this
			*/
			Vec2& operator+=(const Vec2& other);
			
			/**
			* @brief Subtracts other vector's components from this vector's components
			* @param other - vector to be subtracted
			* @return this
			*/
			Vec2& operator-=(const Vec2& other);
			
			/**
			* @brief Multiplies other vector's components by this vector's components
			* @param other - vector to be multiplied
			* @return this
			*/
			Vec2& operator*=(const Vec2& other);
			
			/**
			* @brief Divides other vector's components into this vector's components
			* @param other - vector to be divided by
			* @return this
			*/
			Vec2& operator/=(const Vec2& other); 
			
			/**
			* @brief Multiplies vector by f32 scalar quantity
			* @param scalar - quantity to multiply by
			* @return this
			*/
			Vec2& operator*=(const f32& scalar);

			/**
			* @brief Performs dot product with other vector and returns the calculated value
			* @param other - vector to perform dot product with
			* @return f32 - value of dot product
			*/
			f32 Dot(const Vec2& other) const;

			
			/**
			* @brief Calculates distance from other vector provided
			* @param other - vector used in distance calculation
			* @return f32 - distance between two vectors
			*/
			f32 Distance(const Vec2& other) const;

			/** 
			* @brief Calculates magnitude of this vector
			* @return f32 - magnitude of vector
			*/	
			f32 Length() const;
			
			/**
			* @brief Static function that calculates squared distance from one vector to another
			* @param a - LH vector used in distance calculation
			* @param b - RH vector used in distance calculation
			* @return f32 - squared distance between two vectors
			*/
			static f32 Distance(const Vec2& a, const Vec2& b);
			
			/** 
			* @brief Static function that accepts a vector and returns a new 
			*		vector that is normalized from the one passed in
			* @param vec - the vector to be used for the normalization
			* @return Vec2 - new normalized vector
			*/	
			static Vec2 Normalize(const Vec2& vec);
			
			/** 
			* @brief Static function that performs dot product between two vectors provided
			* @param a - LH vector used in dot product
			* @param b - RH vector used in dot product
			* @return f32 - Dot product between the two vectorsk
			*/	
			static f32 Dot(Vec2& a, Vec2& b);
		
		public: 
			f32 x; 
			f32 y; 
	}; 

	class iVec2
	{
		public:

			/**
			* @brief Default constructor
			*/
			iVec2() = default; 

			/**
			* @brief Constructor that takes s32's for x and y 		
			* @param x - s32 value to be set for x component
			* @param y - s32 value to be set for y component
			*/
			iVec2(const s32& _x, const s32& _y)
				: x(_x), y(_y)
			{
			} 

			/**
			* @brief Constuctor that takes one s32 and assigns it to both x and y
			* @param val - s32 value to be set
			*/
			iVec2(const s32& val)
				: x(val), y(val)
			{
			}
			
			/**
			* @brief Default Destructor
			*/
			~iVec2() {}
			
			/**
			* @brief Stream operator for displaying components of vector
			* @param stream - ostream to be used
			* @param vector - vector to be streamed
			*/
			friend std::ostream& operator<<(std::ostream& stream, iVec2& vector);	

			/**
			* @brief Adds components of this vector to another
			* @param other - vector to be added to this 
			* @return this
			*/
			iVec2& Add(const iVec2& other);
			
			/**
			* @brief Subtracts components of other vector from this
			* @return this
			*/
			iVec2& Subtract(const iVec2& other);
			
			/**
			* @brief Multiplies components of other vector to this
			* @return this
			*/
			iVec2& Multiply(const iVec2& other);
			
			/**
			* @brief Divides components of this vector by other
			* @return this
			*/
			iVec2& Divide(const iVec2& other); 
			
			/**
			* @brief Scales this vector by a f32 scalar
			* @param other - vector to be used as divisor 
			* @return this
			*/
			iVec2& Scale(const s32& scalar);

			/**
			* @brief Adds one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec2 - The sum of the two vectors as new vector
			*/
			friend iVec2 operator+(iVec2 left, const iVec2& other);
			
			/**
			* @brief Subtracts one vector from another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec2 - The difference of the two vectors as new vector
			*/
			friend iVec2 operator-(iVec2 left, const iVec2& other); 
			
			/**
			* @brief Multiplies one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec2 - The product of the two vectors as new vector
			*/
			friend iVec2 operator*(iVec2 left, const iVec2& other);
			
			/**
			* @brief Divides one vector by another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec2 - The quotient of the two vectors as new vector
			*/
			friend iVec2 operator/(iVec2 left, const iVec2& other);
			
			/**
			* @brief Equality operator to check whether two vectors have same components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if same, False if not
			* @return
			*/
			friend b8 operator==(iVec2 left, const iVec2& other);
			
			/**
			* @brief Inequality operator to check whether two vectors have different components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if not same, False if same
			*/
			friend b8 operator!=(iVec2 left, const iVec2& other);
			
			/**
			* @brief Adds other vector's components onto this vector's components
			* @param other - vector to be added
			* @return this
			*/
			iVec2& operator+=(const iVec2& other);
			
			/**
			* @brief Subtracts other vector's components from this vector's components
			* @param other - vector to be subtracted
			* @return this
			*/
			iVec2& operator-=(const iVec2& other);
			
			/**
			* @brief Multiplies other vector's components by this vector's components
			* @param other - vector to be multiplied
			* @return this
			*/
			iVec2& operator*=(const iVec2& other);
			
			/**
			* @brief Divides other vector's components into this vector's components
			* @param other - vector to be divided by
			* @return this
			*/
			iVec2& operator/=(const iVec2& other); 
			
			/**
			* @brief Multiplies vector by f32 scalar quantity
			* @param scalar - quantity to multiply by
			* @return this
			*/
			iVec2& operator*=(const s32& scalar);
			
		public:
			s32 x; 
			s32 y; 
	};
}

typedef Enjon::Vec2 v2;

#endif