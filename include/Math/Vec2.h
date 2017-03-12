#ifndef ENJON_VEC2_H
#define ENJON_VEC2_H

#include <iostream>

namespace Enjon { namespace Math { 

	struct Vec2
	{
		float x; 
		float y;

		Vec2() = default; 
		Vec2(const float& x, const float& y)
		{
			this->x = x;
			this->y = y;
		} 
		Vec2(const float& val)
		{
			this->x = val;
			this->y = val;
		}
		
		friend std::ostream& operator<<(std::ostream& stream, Vec2& vector);	

		Vec2& Add(const Vec2& other);
		Vec2& Subtract(const Vec2& other);
		Vec2& Multiply(const Vec2& other);
		Vec2& Divide(const Vec2& other); 
		Vec2& Scale(const float& scalar);

		friend Vec2 operator+(Vec2 left, const Vec2& other);
		friend Vec2 operator-(Vec2 left, const Vec2& other); 
		friend Vec2 operator*(Vec2 left, const Vec2& other);
		friend Vec2 operator/(Vec2 left, const Vec2& other);
		friend bool operator==(Vec2 left, const Vec2& other);
		//friend bool operator!=(Vec2 left, const Vec2& other);
		
		Vec2& operator+=(const Vec2& other);
		Vec2& operator-=(const Vec2& other);
		Vec2& operator*=(const Vec2& other);
		Vec2& operator/=(const Vec2& other); 
		Vec2& operator*=(const float& scalar);

		float DotProduct(const Vec2& other);

		float Distance(const Vec2& a, const Vec2& b);
		float DistanceTo(const Vec2& other);
		float Length() const;
		
		static Vec2 inline Normalize(const Vec2& vec) { return vec / vec.Length(); }
		static float inline Dot(Vec2& a, Vec2& b) { return a.DotProduct(b); }

	}; 

	struct iVec2
	{
		int x; 
		int y;

		iVec2() = default; 
		iVec2(const int& x, const int& y)
		{
			this->x = x;
			this->y = y;
		} 
		iVec2(const int& val)
		{
			this->x = val;
			this->y = val;
		}
		
		friend std::ostream& operator<<(std::ostream& stream, iVec2& vector);	

		iVec2& Add(const iVec2& other);
		iVec2& Subtract(const iVec2& other);
		iVec2& Multiply(const iVec2& other);
		iVec2& Divide(const iVec2& other); 
		iVec2& Scale(const int& scalar);

		friend iVec2 operator+(iVec2 left, const iVec2& other);
		friend iVec2 operator-(iVec2 left, const iVec2& other); 
		friend iVec2 operator*(iVec2 left, const iVec2& other);
		friend iVec2 operator/(iVec2 left, const iVec2& other);
		friend bool operator==(iVec2 left, const iVec2& other);
		friend bool operator!=(iVec2 left, const iVec2& other);
		
		iVec2& operator+=(const iVec2& other);
		iVec2& operator-=(const iVec2& other);
		iVec2& operator*=(const iVec2& other);
		iVec2& operator/=(const iVec2& other); 
		iVec2& operator*=(const int& scalar);

	};
}}

typedef Enjon::Math::Vec2 v2;

#endif