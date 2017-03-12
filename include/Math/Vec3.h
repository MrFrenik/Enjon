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
		Vec3(const float& _x, const float& _y, const float& _z)
			: x(_x), y(_y), z(_z) 
		{
		}
		
		Vec3(const Vec2& a, const float& val)
			: x(a.x), y(a.y), z(val)
		{
		}

		Vec3(const float& val)
			: x(val), y(val), z(val)
		{
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
		friend Vec3 operator*(const float& scalar, Vec3 right);
		friend void operator*=(Vec3& left, const float& scalar);
		friend void operator*=(const float& scalar, Vec3& left);
		friend Vec3 operator/(Vec3 left, const Vec3& other);
		friend Vec3 operator/(Vec3 left, float value);
		friend bool operator==(const Vec3& left, const Vec3& right); 
		friend bool operator!=(const Vec3& left, const Vec3& right);
		
		Vec3& operator+=(const Vec3& other);
		Vec3& operator-=(const Vec3& other);
		Vec3& operator*=(const Vec3& other);
		Vec3& operator/=(const Vec3& other);
		Vec3 operator-();

		float Dot(const Vec3& other);
		Vec3 Cross(const Vec3& other);

		inline static Vec3 XAxis() { return Vec3(1.0f, 0.0f, 0.0f); }
		inline static Vec3 YAxis() { return Vec3(0.0f, 1.0f, 0.0f); }
		inline static Vec3 ZAxis() { return Vec3(0.0f, 0.0f, 1.0f); } 
	
		float Vec3::Length() const; 

		static Vec3 inline Normalize(const Vec3& vec) { return vec / vec.Length(); }
		static float inline DistanceSquared(const Vec3& a, const Vec3& b)
		{
			float x = a.x - b.x;
			float y = a.y - b.y;
			float z = a.z - b.z;

			return x*x + y*y + z*z;
		}
	}; 
		

}}

typedef Enjon::Math::Vec3 v3;

#endif