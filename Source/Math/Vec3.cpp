// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Vec2.cpp

#include "Math/Vec3.h"
#include "Math/Quaternion.h"

namespace Enjon 
{ 
	//==================================================================

	Vec2 Vec3::XY() const 
	{ 
		return Vec2(this->x, this->y); 
	} 
	
	//================================================

	Vec3 Vec3::operator-()
	{
		return *this * -1.0f;
	}
	
	//================================================

	b8 operator==(const Vec3& left, const Vec3& right) 
	{
		return (left.x == right.x && left.y == right.y && left.z == right.z);
	}
	
	//================================================

	b8 operator!=(const Vec3& left, const Vec3& right) 
	{
		return !(left == right);
	}
	
	//================================================

	std::ostream& operator<<(std::ostream& stream, Vec3& vector)
	{
		stream << "Vector3f: (" << vector.x << ", " << vector.y << ", " << vector.z << ")";
		return stream;
	}
			
	Vec3 Vec3::operator*(const Quaternion& quat) const
	{
		//return quat.Rotate( *this );
		Vec3 t = *this;
		auto Qxyz = Vec3(quat.x, quat.y, quat.z);
		Vec3 T = 2.0f * t.Cross(Qxyz);
		return (t + quat.w * T + Qxyz.Cross(T));
	} 
	
	//================================================

	Vec3& Vec3::Add(const Vec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	} 
	
	//================================================
	
	Vec3& Vec3::Subtract(const Vec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	
	//================================================
	
	Vec3& Vec3::Multiply(const Vec3& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}
	
	//================================================
	
	Vec3& Vec3::Divide(const Vec3& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}
	
	//================================================

	Vec3 operator+(Vec3 left, const Vec3& right)
	{
		return left.Add(right);
	}
	
	//================================================
	
	Vec3 operator-(Vec3 left, const Vec3& right)
	{
		return left.Subtract(right);
	}
	
	//================================================
	
	Vec3 operator*(Vec3 left, const Vec3& right)
	{
		return left.Multiply(right);
	}
	
	//================================================
		
	Vec3 operator*(Vec3 left, const f32& scalar)
	{
		return Vec3(left.x * scalar, left.y * scalar, left.z * scalar);
	}
	
	//================================================

	Vec3 operator*(const f32& scalar, Vec3 right)
	{
		return operator*(right, scalar);
	}
	
	//================================================

	void operator*=(Vec3& left, const f32& scalar)
	{
		left.x *= scalar;
		left.y *= scalar;
		left.z *= scalar;
	}
	
	//================================================

	void operator*=(const f32& scalar, Vec3 left)
	{
		operator*=(left, scalar);
	}
	
	//================================================

	Vec3 operator/(Vec3 left, const Vec3& right)
	{
		return left.Divide(right);
	}
	
	//================================================
		
	Vec3 operator/(Vec3 left, const f32& value)
	{
		return Vec3(left.x / value, left.y / value, left.z / value);
	}
	
	//================================================

	Vec3& Vec3::operator+=(const Vec3& other)
	{
		return Add(other);
	} 
	
	//================================================
	
	Vec3& Vec3::operator-=(const Vec3& other)
	{
		return Subtract(other);
	} 
	
	//================================================
	
	Vec3& Vec3::operator*=(const Vec3& other)
	{
		return Multiply(other);
	} 
	
	//================================================
	
	Vec3& Vec3::operator/=(const Vec3& other)
	{
		return Divide(other);
	} 
	
	//================================================
	
	f32 Vec3::Length() const 
	{
		return sqrt( this->Length2() );
	}
	
	//================================================

	f32 Vec3::Length2( ) const
	{
		return this->Dot( *this );
	}
	
	//================================================

	f32 Vec3::Dot(const Vec3& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}
	
	//================================================

	Vec3 Vec3::Cross(const Vec3& other) const
	{
		return Vec3(
					y * other.z - z * other.y, 
					z * other.x - x * other.z, 
					x * other.y - y * other.x
					);	
	} 
	
	//================================================
			
	Vec3 Vec3::Normalize(const Vec3& vec)
	{ 
		f32 length = vec.Length( );
		return length != 0.0f ? vec / length : vec; 
	}
 
	//================================================
			
	Vec3 Vec3::Normalize( ) const
	{
		f32 length = Length( );
		return length != 0.0f ? *this / length : *this;
	}
	
	//==================================================================

	f32 Vec3::Distance(const Vec3& other) const
	{
		return std::sqrtf(std::pow(other.x - x, 2) + std::pow(other.y - y, 2) + std::pow(other.z - z, 2));
	}
	
	//================================================
			
	f32 Vec3::DistanceSquared(const Vec3& a, const Vec3& b)
	{
		f32 x = a.x - b.x;
		f32 y = a.y - b.y;
		f32 z = a.z - b.z;

		return x*x + y*y + z*z;
	} 
			
	Vec3 Vec3::XAxis()
	{ 
		return Vec3(1.0f, 0.0f, 0.0f); 
	}
	
	//================================================
			
	Vec3 Vec3::YAxis() 
	{ 
		return Vec3(0.0f, 1.0f, 0.0f); 
	}
	
	//================================================
	
	Vec3 Vec3::ZAxis() 
	{ 
		return Vec3(0.0f, 0.0f, 1.0f); 
	} 
	
	//================================================
			
	std::ostream& operator<<(std::ostream& stream, iVec3& vector)
	{
		stream << "Vector3i: (" << vector.x << ", " << vector.y << ", " << vector.z << ")";
		return stream; 
	}
	
	//================================================

	iVec3& iVec3::Add(const iVec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this; 
	}
	
	//================================================
	
	iVec3& iVec3::Subtract(const iVec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this; 
	}
	
	//================================================
	
	iVec3& iVec3::Multiply(const iVec3& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this; 
	}
	
	//================================================
	
	iVec3& iVec3::Divide(const iVec3& other)
	{ 
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this; 
	}
	
	//================================================
	
	iVec3& iVec3::Scale(const s32& scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	
	//================================================

	iVec3 operator+(iVec3 left, const iVec3& right)
	{
		return left.Add(right); 
	}
	
	//================================================
	
	iVec3 operator-(iVec3 left, const iVec3& right)
	{
		return left.Subtract(right); 
	}
	
	//================================================
	
	iVec3 operator*(iVec3 left, const iVec3& right)
	{
		return left.Multiply(right); 
	}
	
	//================================================
	
	iVec3 operator/(iVec3 left, const iVec3& right)
	{
		return left.Divide(right);
	}
	
	//================================================
	
	b8 operator==(iVec3 left, const iVec3& right)
	{ 
		return (left.x == right.x && left.y == right.y && left.z == right.z);
	}
	
	//================================================
	
	b8 operator!=(iVec3 left, const iVec3& right)
	{
		return !(left == right);
	}
	
	//================================================

	iVec3& iVec3::operator+=(const iVec3& other)
	{
		return Add(other); 
	}
	
	//================================================
	
	iVec3& iVec3::operator-=(const iVec3& other)
	{ 
		return Subtract(other); 
	}
	
	//================================================
	
	iVec3& iVec3::operator*=(const iVec3& other)
	{
		return Multiply(other);
	}
	
	//================================================
	
	iVec3& iVec3::operator/=(const iVec3& other)
	{
		return Divide(other); 
	}
	
	//================================================
	
	iVec3& iVec3::operator*=(const s32& scalar)
	{
		return Multiply(iVec3(scalar));
	}
}