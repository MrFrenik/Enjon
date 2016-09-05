#include "math/Vec3.h"

namespace Enjon { namespace Math { 
	
	std::ostream& operator<<(std::ostream& stream, Vec3& vector)
	{
		stream << "Vector3f: (" << vector.x << ", " << vector.y << ", " << vector.z << ")";
		return stream;
	}

	Vec3& Vec3::Add(const Vec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	} 
	
	Vec3& Vec3::Subtract(const Vec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	
	Vec3& Vec3::Multiply(const Vec3& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}
	
	Vec3& Vec3::Divide(const Vec3& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	Vec3 operator+(Vec3 left, const Vec3& right)
	{
		return left.Add(right);
	}
	
	Vec3 operator-(Vec3 left, const Vec3& right)
	{
		return left.Subtract(right);
	}
	
	Vec3 operator*(Vec3 left, const Vec3& right)
	{
		return left.Multiply(right);
	}
		
	Vec3 operator*(Vec3 left, const float& scalar)
	{
		return Vec3(left.x * scalar, left.y * scalar, left.z * scalar);
	}
	
	Vec3 operator/(Vec3 left, const Vec3& right)
	{
		return left.Divide(right);
	}

	Vec3& Vec3::operator+=(const Vec3& other)
	{
		return Add(other);
	} 
	
	Vec3& Vec3::operator-=(const Vec3& other)
	{
		return Subtract(other);
	} 
	
	Vec3& Vec3::operator*=(const Vec3& other)
	{
		return Multiply(other);
	} 
	
	Vec3& Vec3::operator/=(const Vec3& other)
	{
		return Divide(other);
	} 
	
	float Vec3::Length() const 
	{
		return sqrt((x * x) + (y * y) + (z * z));
	}
		
	Vec3 operator/(Vec3 left, float value)
	{
		return Vec3(left.x / value, left.y / value, left.z / value);
	}

	float Vec3::DotProduct(const Vec3& other)
	{
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 Vec3::CrossProduct(const Vec3& other)
	{
		return Vec3(
					y * other.z - z * other.y, 
					z * other.x - x * other.z, 
					x * other.y - y * other.x
					);	
	}

}}