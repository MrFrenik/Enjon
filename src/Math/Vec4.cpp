#include "math/Vec4.h"

namespace Enjon { namespace Math { 
 
	std::ostream& operator<<(std::ostream& stream, Vec4& vector)
	{
		stream << "Vector4f: (" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
		return stream;
	}

	Vec4& Vec4::Add(const Vec4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	} 
	
	Vec4& Vec4::Subtract(const Vec4& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}
	
	Vec4& Vec4::Multiply(const Vec4& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}
	
	Vec4& Vec4::Divide(const Vec4& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	bool operator==(const Vec4& left, const Vec4& right) 
	{
		return (left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w);
	}

	bool operator!=(const Vec4& left, const Vec4& right) 
	{
		return !(left == right);
	}

	Vec4 operator+(Vec4 left, const Vec4& right)
	{
		return left.Add(right);
	}
	
	Vec4 operator-(Vec4 left, const Vec4& right)
	{
		return left.Subtract(right);
	}
	
	Vec4 operator*(Vec4 left, const Vec4& right)
	{
		return left.Multiply(right);
	}
	
	Vec4 operator/(Vec4 left, const Vec4& right)
	{
		return left.Divide(right);
	}

	Vec4& Vec4::operator+=(const Vec4& other)
	{
		return Add(other);
	} 
	
	Vec4& Vec4::operator-=(const Vec4& other)
	{
		return Subtract(other);
	} 
	
	Vec4& Vec4::operator*=(const Vec4& other)
	{
		return Multiply(other);
	} 
	
	Vec4& Vec4::operator/=(const Vec4& other)
	{
		return Divide(other);
	} 

	float Vec4::Length() const 
	{
		return sqrt((x * x) + (y * y) + (z * z) + (w * w));
	}
		
	Vec4 operator/(Vec4 left, float value)
	{
		return Vec4(left.x / value, left.y / value, left.z / value, left.w / value);
	}

	float Vec4::Dot(const Vec4& other)
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}
}}