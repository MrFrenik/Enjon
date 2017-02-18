#include "math/Vec2.h"

namespace Enjon { namespace Math { 

	std::ostream& operator<<(std::ostream& stream, Vec2& vector)
	{
		stream << "Vector2f: (" << vector.x << ", " << vector.y << ")";
		return stream;
	}

	Vec2& Vec2::Add(const Vec2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	} 
	
	Vec2& Vec2::Subtract(const Vec2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	
	Vec2& Vec2::Multiply(const Vec2& other)
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	
	Vec2& Vec2::Divide(const Vec2& other)
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}
	
	Vec2& Vec2::Scale(const float& scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	Vec2 operator+(Vec2 left, const Vec2& right)
	{
		return left.Add(right);
	}
	
	Vec2 operator-(Vec2 left, const Vec2& right)
	{
		return left.Subtract(right);
	}
	
	Vec2 operator*(Vec2 left, const Vec2& right)
	{
		return left.Multiply(right);
	}
	
	Vec2 operator/(Vec2 left, const Vec2& right)
	{
		return left.Divide(right);
	}

	Vec2 operator*(Vec2 left, const float& scalar)
	{
		return left.Scale(scalar);
	}
	
	bool operator==(Vec2 left, const Vec2& other)
	{
		return (left.x == other.x && left.y == other.y);
	}
	//
	//Vec2 operator!=(Vec2 left, const Vec2& other)
	//{
	//	return (left.x != other.x || left.y != other.y);
	//}

	Vec2& Vec2::operator+=(const Vec2& other)
	{
		return Add(other);
	} 
	
	Vec2& Vec2::operator-=(const Vec2& other)
	{
		return Subtract(other);
	} 
	
	Vec2& Vec2::operator*=(const Vec2& other)
	{
		return Multiply(other);
	} 
	
	Vec2& Vec2::operator/=(const Vec2& other)
	{
		return Divide(other);
	} 
	
	Vec2& Vec2::operator*=(const float& scalar)
	{
		return Scale(scalar);
	}

	//DON'T BE STUPID AND MAKE THESE TEMPLATES...
	
	std::ostream& operator<<(std::ostream& stream, iVec2& vector)
	{
		stream << "Vector2i: (" << vector.x << ", " << vector.y << ")";
		return stream;
	}

	iVec2& iVec2::Add(const iVec2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	} 
	
	iVec2& iVec2::Subtract(const iVec2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	
	iVec2& iVec2::Multiply(const iVec2& other)
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}
	
	iVec2& iVec2::Divide(const iVec2& other)
	{
		x = (int)(x / other.x);
		y = (int)(y / other.y);
		return *this;
	}
	
	iVec2& iVec2::Scale(const int& scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	iVec2 operator+(iVec2 left, const iVec2& right)
	{
		return left.Add(right);
	}
	
	iVec2 operator-(iVec2 left, const iVec2& right)
	{
		return left.Subtract(right);
	}
	
	iVec2 operator*(iVec2 left, const iVec2& right)
	{
		return left.Multiply(right);
	}
	
	iVec2 operator/(iVec2 left, const iVec2& right)
	{
		return left.Divide(right);
	}

	iVec2 operator*(iVec2 left, const int& scalar)
	{
		return left.Scale(scalar);
	}

	iVec2& iVec2::operator+=(const iVec2& other)
	{
		return Add(other);
	} 
	
	iVec2& iVec2::operator-=(const iVec2& other)
	{
		return Subtract(other);
	} 
	
	iVec2& iVec2::operator*=(const iVec2& other)
	{
		return Multiply(other);
	} 
	
	iVec2& iVec2::operator/=(const iVec2& other)
	{
		return Divide(other);
	} 
	
	iVec2& iVec2::operator*=(const int& scalar)
	{
		return Scale(scalar);
	}

	bool operator==(iVec2 left, const iVec2& other)
	{
		return ((left.x == other.x) && (left.y == other.y));
	}

	bool operator!=(iVec2 left, const iVec2& other)
	{
		return ((left.x != other.x) || (left.y != other.y));
	}

	float Vec2::DotProduct(const Vec2& other)
	{
		return (x * other.x) + (y * other.y);
	}

	float Vec2::DistanceTo(const Vec2& other)
	{
		return sqrt(pow(other.x - x,2)+pow(other.y - y,2));
	}

	float Vec2::Length() const 
	{
		return sqrt((x * x) + (y * y));
	}

}}