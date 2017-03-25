// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Quaternion.cpp

#include <Math/Quaternion.h>

namespace Enjon 
{ 
	Vec3 Quaternion::XYZ()
	{
		return Vec3(x, y, z);
	}

	f32 Quaternion::Length()
	{
		return sqrt(x*x + y*y + z*z + w*w);
	}
	
	Quaternion Quaternion::Normalize()
	{
		Quaternion Q = *this;
		auto L = this->Length();

		Q.x /= L;
		Q.y /= L;
		Q.z /= L;
		Q.w /= L;

		return Q;
	}

	Quaternion Quaternion::Conjugate() const
	{
		Quaternion Q = *this;
		// Quaternion Q;
		Q.x *= -1;
		Q.y *= -1;
		Q.z *= -1;
		return Q;
	}

	// Negative unary operator overload
	Quaternion Quaternion::operator-() const
	{
		return *this * -1.0f;
	}

	Quaternion Quaternion::operator+(const Quaternion& Other) const
	{
		Quaternion C;

		C.x = x + Other.x;
		C.y = y + Other.y;
		C.z = z + Other.z;
		C.w = w + Other.w;

		return C;
	}

	Quaternion Quaternion::operator-(const Quaternion& Other) const
	{
		Quaternion C;

		C.x = x - Other.x;
		C.y = y - Other.y;
		C.z = z - Other.z;
		C.w = w - Other.w;

		return C;
	}

	Quaternion Quaternion::operator*(const Quaternion& Other) const 
	{
		Quaternion Result;
		
		Result.x = w*Other.x + x*Other.w + y*Other.z - z*Other.y;
		Result.y = w*Other.y - x*Other.z + y*Other.w + z*Other.x;
		Result.z = w*Other.z + x*Other.y - y*Other.x + z*Other.w;
		Result.w = w*Other.w - x*Other.x - y*Other.y - z*Other.z;

		return Result;	
	}

	Quaternion Quaternion::operator*(const f32& V) const
	{
		Quaternion C = *this;

		C.x *= V;
		C.y *= V;
		C.z *= V;
		C.w *= V;

		return C;
	}


	Quaternion Quaternion::operator/(const f32& V) const
	{
		Quaternion C = *this;
		
		C.x /= V;
		C.y /= V;
		C.z /= V;
		C.w /= V;

		return C;	
	}

	bool Quaternion::operator==(const Quaternion& Other) const
	{
		if (Other.x != x) return false;
		if (Other.y != y) return false;
		if (Other.z != z) return false;
		if (Other.w != w) return false;

		return true;		
	}

	bool Quaternion::operator!=(const Quaternion& Other) const
	{
		return !(Other == *this);
	}

	std::ostream& operator<<(std::ostream& stream, const Quaternion& Q)
	{
		stream << "Quaternion: (" << Q.x << ", " << Q.y << ", " << Q.z << ", " << Q.w << ")";
		return stream;	
	}

	f32 Quaternion::Dot(Quaternion& Q)
	{
		auto A = Vec3(Q.x, Q.y, Q.z);
		auto B = Vec3(x, y, z);

		return A.Dot(B) + w * Q.w;	
	}

	// Returns cross product with another quaternion
	Quaternion Quaternion::Cross(const Quaternion& Other) const
	{
		Quaternion C;

		C.x = w*Other.x + x*Other.w + y*Other.z - z*Other.y;
		C.y = w*Other.y + y*Other.w + z*Other.x - x*Other.z;
		C.z = w*Other.z + z*Other.w + x*Other.y - y*Other.x;
		C.w = w*Other.w - x*Other.x - y*Other.y - z*Other.z; 		

		return C;	
	}

	Quaternion Quaternion::Inverse(Quaternion& Q)
	{
		Quaternion C = Q.Conjugate() / Q.Dot(Q);
		return C;
	}

	f32 Quaternion::Angle()
	{
		return 2.0f * std::acos(w);
	}

	Vec3 Quaternion::Axis()
	{
		// 1 - cos(theta/2)^2 = sin(theta/2);
		f32 S2 = 1.0f - w * w;

		// If negative, return
		if (S2 <= 0.0f) return Vec3(0.0f, 0.0f, 1.0f);

		f32 InverseS2 = 1.0f / std::sqrt(S2);

		return Vec3(x, y, z) * InverseS2;
	}

	f32 Quaternion::Roll()
	{
		return std::atan2(2.0f*x*y + z*w, x*x + w*w - y*y - z*z);
	}

	f32 Quaternion::Pitch()
	{
		return std::atan2(2.0f*y*z + w*x, w*w - x*x - y*y + z*z);
	}

	f32 Quaternion::Yaw()
	{
		return std::asin(-2.0f * (x*z - w*y));
	}

	Vec3 Quaternion::EulerAngles()
	{
		return Vec3(this->Pitch(), this->Yaw(), this->Roll());
	} 

	Vec3 Quaternion::operator*(const Vec3& V) const
	{
		auto Qxyz = Vec3(x, y, z);
		Vec3 T = 2.0f * Qxyz.Cross(V);
		return (V + w * T + Qxyz.Cross(T));
	} 
}

























