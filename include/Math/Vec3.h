#ifndef ENJON_VEC3_H
#define ENJON_VEC3_H

#include <iostream>

#include "Math/Vec2.h"

#define XAXIS       Vec3(1.0f, 0.0f, 0.0f)
#define YAXIS       Vec3(0.0f, 1.0f, 0.0f)
#define ZAXIS       Vec3(0.0f, 0.0f, 1.0f)

namespace Enjon { namespace Math { 

	struct Vec3
	{ 
		float x; 
		float y;
		float z;

		Vec3() = default;
		Vec3(const float& x, const float& y, const float& z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			
		}
		Vec3(const Vec2& a, const float& val)
		{
			this->x = a.x;
			this->y = a.y;
			this->z = val;
		}

		Vec2 inline XY() const { return Vec2(this->x, this->y); }
	
		friend std::ostream& operator<<(std::ostream& stream, Vec3& vector);	

		Vec3& Add(const Vec3& other);
		Vec3& Subtract(const Vec3& other);
		Vec3& Multiply(const Vec3& other);
		Vec3& Divide(const Vec3& other); 

		friend Vec3 operator+(Vec3 left, const Vec3& other);
		friend Vec3 operator-(Vec3 left, const Vec3& other); 
		friend Vec3 operator*(Vec3 left, const Vec3& other);
		friend Vec3 operator*(Vec3 left, const float& scalar);
		friend Vec3 operator/(Vec3 left, const Vec3& other);
		friend Vec3 operator/(Vec3 left, float value);
		
		Vec3& operator+=(const Vec3& other);
		Vec3& operator-=(const Vec3& other);
		Vec3& operator*=(const Vec3& other);
		Vec3& operator/=(const Vec3& other);

		float DotProduct(const Vec3& other);
		Vec3 CrossProduct(const Vec3& other);

		inline static Vec3 XAxis() { return Vec3(1.0f, 0.0f, 0.0f); }
		inline static Vec3 YAxis() { return Vec3(0.0f, 1.0f, 0.0f); }
		inline static Vec3 ZAxis() { return Vec3(0.0f, 0.0f, 1.0f); } 
	
		float Vec3::Length() const; 

		static Vec3 inline Normalize(const Vec3& vec) { return vec / vec.Length(); }
	}; 
		

}}

#endif