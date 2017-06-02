#ifndef ENJON_VEC4_H
#define ENJON_VEC4_H

#include <iostream>
#include "Math/Vec2.h"
#include "Math/Vec3.h"

namespace Enjon {

	class Vec4
	{ 
		public: 

			/**
			* @brief Default constructor
			*/
			Vec4() = default;

			/**
			* @brief Constructor that takes f32's for x, y, z, w
			* @param x - f32 value to be set for x component
			* @param y - f32 value to be set for y component
			* @param z - f32 value to be set for y component
			* @param w - f32 value to be set for y component
			*/
			Vec4(const f32& _x, const f32& _y, const f32& _z, const f32& _w)
				: x(_x), y(_y), z(_z), w(_w)
			{
			}

			/**
			* @brief Constructor that takes f32's for x, y, and vec2 for z, w
			* @param x - f32 value to be set for x component
			* @param y - f32 value to be set for y component
			* @param a - vec2 to be used to set z, w components
			*/
			Vec4(const f32& _x, const f32& _y, const Vec2& a)
				: x(_x), y(_y), z(a.x), w(a.y)
			{
			}

			/**
			* @brief Constructor that takes f32's for z, w, and vec2 for x, y
			* @param a - vec2 to be used to set x, y components
			* @param x - f32 value to be set for z component
			* @param y - f32 value to be set for w component
			*/
			Vec4(const Vec2& a, const f32& _z, const f32& _w)
				: x(a.x), y(a.y), z(_z), w(_w)
			{
			}

			/**
			* @brief Constructor that takes f32's for x, y, z, w
			* @param a - vec2 to be used to set x, y components
			* @param a - vec2 to be used to set z, w components
			*/
			Vec4(const Vec2& a, const Vec2& b)
				: x(a.x), y(a.y), z(b.x), w(b.y)
			{
			}

			/**
			* @brief Constructor that takes vec3 for x, y, z and f32 for w
			* @param a - vec3 to be used to set x, y, z components
			* @param a - f32 to be used to setw components
			*/
			Vec4(const Vec3& a, const f32& _w)
				: x(a.x), y(a.y), z(a.z), w(_w)
			{
			} 
			
			/**
			* @brief Constructor that takes f32's for x, y, z, w
			* @param val - f32 value to be set for all components

			*/
			Vec4(const f32& val)
				: x(val), y(val), z(val), w(val)
			{
			}
			
			friend std::ostream& operator<<(std::ostream& stream, Vec4& vector);	

			Vec4& Add(const Vec4& other);
			Vec4& Subtract(const Vec4& other);
			Vec4& Multiply(const Vec4& other);
			Vec4& Divide(const Vec4& other); 

			friend Vec4 operator+(Vec4 left, const Vec4& other);
			friend Vec4 operator-(Vec4 left, const Vec4& other); 
			friend Vec4 operator*(Vec4 left, const Vec4& other);
			friend Vec4 operator/(Vec4 left, const Vec4& other);

			friend Vec4 operator*(Vec4 left, const f32& scalar);
			friend Vec4 operator*(const f32& scalar, Vec4 right);
			friend void operator*=(Vec4& left, const f32& scalar);
			friend Vec4 operator*=(const f32& scalar, Vec4& left);
			friend Vec4 operator/(Vec4 left, const Vec4& other);
			friend Vec4 operator/(Vec4 left, f32 value);
			friend void operator/=(Vec4& left, f32 scalar);
			friend bool operator==(const Vec4& left, const Vec4& right); 
			friend bool operator!=(const Vec4& left, const Vec4& right);
			
			Vec4& operator+=(const Vec4& other);
			Vec4& operator-=(const Vec4& other);
			Vec4& operator*=(const Vec4& other);
			Vec4& operator/=(const Vec4& other);

			f32 Vec4::Length() const;
			f32 Vec4::Dot(const Vec4& other); 
			Vec3 XYZ(); 
			
			static Vec4 Normalize(const Vec4& vec);
			
		public:

			f32 x; 
			f32 y;
			f32 z;
			f32 w; 
	};
}

typedef Enjon::Vec4 v4;

#endif