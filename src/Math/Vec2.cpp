// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Vec2.cpp

#include "Math/Vec2.h"

namespace Enjon 
{  
	//==================================================================

	std::ostream& operator<<(std::ostream& stream, Vec2& vector)
	{
		stream << "Vector2f: (" << vector.x << ", " << vector.y << ")";
		return stream;
	}

	//==================================================================

	Vec2& Vec2::Add(const Vec2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	} 
	
	//==================================================================

	Vec2& Vec2::Subtract(const Vec2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	
	//==================================================================

	Vec2& Vec2::Multiply(const Vec2& other)
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	
	//==================================================================

	Vec2& Vec2::Divide(const Vec2& other)
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}
	
	//==================================================================

	Vec2& Vec2::Scale(const f32& scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	//==================================================================

	Vec2 operator+(Vec2 left, const Vec2& right)
	{
		return left.Add(right);
	}
	
	//==================================================================

	Vec2 operator-(Vec2 left, const Vec2& right)
	{
		return left.Subtract(right);
	}
	
	//==================================================================

	Vec2 operator*(Vec2 left, const Vec2& right)
	{
		return left.Multiply(right);
	}
	
	//==================================================================

	Vec2 operator/(Vec2 left, const Vec2& right)
	{
		return left.Divide(right);
	}

	//==================================================================

	Vec2 operator*(Vec2 left, const f32& scalar)
	{
		return left.Scale(scalar);
	}
	
	//==================================================================

	b8 operator==(Vec2 left, const Vec2& other)
	{
		return (left.x == other.x && left.y == other.y);
	}
	
	//==================================================================

	b8 operator!=(Vec2 left, const Vec2& other)
	{
		return (left.x != other.x || left.y != other.y);
	}

	//==================================================================

	Vec2& Vec2::operator+=(const Vec2& other)
	{
		return Add(other);
	} 
	
	//==================================================================

	Vec2& Vec2::operator-=(const Vec2& other)
	{
		return Subtract(other);
	} 
	
	//==================================================================

	Vec2& Vec2::operator*=(const Vec2& other)
	{
		return Multiply(other);
	} 
	
	//==================================================================

	Vec2& Vec2::operator/=(const Vec2& other)
	{
		return Divide(other);
	} 
	
	//==================================================================

	Vec2& Vec2::operator*=(const f32& scalar)
	{
		return Scale(scalar);
	}

	//==================================================================

	f32 Vec2::Dot(const Vec2& other) const
	{
		return (x * other.x) + (y * other.y);
	}

	//==================================================================

	f32 Vec2::Distance(const Vec2& other) const
	{
		return std::sqrtf(std::pow(other.x - x, 2) + std::pow(other.y - y, 2));
	}

	//==================================================================

	f32 Vec2::Length() const
	{
		return std::sqrtf((x * x) + (y * y));
	} 
	
	//==================================================================

	std::ostream& operator<<(std::ostream& stream, iVec2& vector)
	{
		stream << "Vector2i: (" << vector.x << ", " << vector.y << ")";
		return stream;
	}

	//==================================================================

	iVec2& iVec2::Add(const iVec2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	} 
	
	//==================================================================

	iVec2& iVec2::Subtract(const iVec2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	
	//==================================================================

	iVec2& iVec2::Multiply(const iVec2& other)
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	
	//==================================================================

	iVec2& iVec2::Divide(const iVec2& other)
	{
		x = (s32)(x / other.x);
		y = (s32)(y / other.y);
		return *this;
	}
	
	//==================================================================

	iVec2& iVec2::Scale(const s32& scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	//==================================================================

	iVec2 operator+(iVec2 left, const iVec2& right)
	{
		return left.Add(right);
	}
	
	//==================================================================

	iVec2 operator-(iVec2 left, const iVec2& right)
	{
		return left.Subtract(right);
	}
	
	//==================================================================

	iVec2 operator*(iVec2 left, const iVec2& right)
	{
		return left.Multiply(right);
	}
	
	//==================================================================

	iVec2 operator/(iVec2 left, const iVec2& right)
	{
		return left.Divide(right);
	}

	//==================================================================

	iVec2 operator*(iVec2 left, const s32& scalar)
	{
		return left.Scale(scalar);
	}

	//==================================================================

	iVec2& iVec2::operator+=(const iVec2& other)
	{
		return Add(other);
	} 
	
	//==================================================================

	iVec2& iVec2::operator-=(const iVec2& other)
	{
		return Subtract(other);
	} 
	
	//==================================================================

	iVec2& iVec2::operator*=(const iVec2& other)
	{
		return Multiply(other);
	} 
	
	//==================================================================

	iVec2& iVec2::operator/=(const iVec2& other)
	{
		return Divide(other);
	} 
	
	//==================================================================

	iVec2& iVec2::operator*=(const s32& scalar)
	{
		return Scale(scalar);
	}

	//==================================================================

	b8 operator==(iVec2 left, const iVec2& other)
	{
		return ((left.x == other.x) && (left.y == other.y));
	}

	//==================================================================

	b8 operator!=(iVec2 left, const iVec2& other)
	{
		return ((left.x != other.x) || (left.y != other.y));
	} 

	//==================================================================
			
	Vec2 Vec2::Normalize(const Vec2& vec)
	{ 
		return vec / vec.Length(); 
	}

	//==================================================================
			
	f32 Vec2::Dot(Vec2& a, Vec2& b) 
	{ 
		return a.Dot(b); 
	}
}



