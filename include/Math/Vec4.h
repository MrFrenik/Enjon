#ifndef ENJON_VEC4_H
#define ENJON_VEC4_H

#include <iostream>
#include "Math/Vec2.h"
#include "Math/Vec3.h"

namespace Enjon { namespace Math { 

	struct Vec4
	{ 
		float x; 
		float y;
		float z;
		float w; 

		Vec4() = default;
		Vec4(const float& x, const float& y, const float& z, const float& w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		Vec4(const float& val1, const float& val2, const Vec2& a)
		{
			this->x = val1;
			this->y = val2;
			this->z = a.x;
			this->w = a.y;
		}

		Vec4(const Vec2& a, const float& val1, const float& val2)
		{
			this->x = a.x;
			this->y = a.y;
			this->z = val1;
			this->w = val2;
		}
		Vec4(const Vec2& a, const Vec2& b)
		{
			this->x = a.x;
			this->y = a.y;
			this->z = b.x;
			this->w = b.y;
		}
		Vec4(const Math::Vec3& a, const float& val)
		{
			this->x = a.x;
			this->y = a.y;
			this->z = a.z;
			this->w = val;
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

		friend Vec4 operator*(Vec4 left, const float& scalar);
		friend Vec4 operator*(const float& scalar, Vec4 right);
		friend void operator*=(Vec4& left, const float& scalar);
		friend Vec4 operator*=(const float& scalar, Vec4& left);
		friend Vec4 operator/(Vec4 left, const Vec4& other);
		friend Vec4 operator/(Vec4 left, float value);
		friend void operator/=(Vec4& left, float scalar);
		friend bool operator==(const Vec4& left, const Vec4& right); 
		friend bool operator!=(const Vec4& left, const Vec4& right);
		
		Vec4& operator+=(const Vec4& other);
		Vec4& operator-=(const Vec4& other);
		Vec4& operator*=(const Vec4& other);
		Vec4& operator/=(const Vec4& other);

		float Vec4::Length() const;
		float Vec4::Dot(const Vec4& other);
		static Vec4 inline Normalize(const Vec4& vec) { return vec / vec.Length(); }
		Vec3 XYZ();

	};
}}

typedef Enjon::Math::Vec4 v4;

#endif