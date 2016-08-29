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
		Vec4(const float& val1, const float& val2, const Math::Vec2& a)
		{
			this->x = val1;
			this->y = val2;
			this->z = a.x;
			this->w = a.y;
		}

		Vec4(const Math::Vec2& a, const float& val1, const float& val2)
		{
			this->x = a.x;
			this->y = a.y;
			this->z = val1;
			this->w = val2;
		}
		Vec4(const Math::Vec2& a, const Math::Vec2& b)
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
		
		Vec4& operator+=(const Vec4& other);
		Vec4& operator-=(const Vec4& other);
		Vec4& operator*=(const Vec4& other);
		Vec4& operator/=(const Vec4& other);
	}; 
		

}}

#endif