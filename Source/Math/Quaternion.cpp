// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Quaternion.cpp

#include <Math/Quaternion.h>

#include <glm/glm.hpp>

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
		return Quaternion( -x, -y, -z, w );
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

		Result.x = w * Other.x + Other.w * x + y * Other.z - Other.y * z;
		Result.y = w * Other.y + Other.w * y + z * Other.x - Other.z * x;
		Result.z = w * Other.z + Other.w * z + x * Other.y - Other.x * y;
		Result.w = w * Other.w - x * Other.x - y * Other.y - z * Other.z; 

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

	Vec3 Quaternion::Rotate( const Vec3& v ) const
	{
		// nVidia SDK implementation

		Vec3 uv;
		Vec3 uuv;
		Vec3 qVec( this->x, this->y, this->z );
		uv = qVec.Cross( v );
		uuv = qVec.Cross( uv );
		uv *= ( 2.0f * w );
		uuv *= 2.0f;

		return v + uv + uuv;
	}

	f32 Quaternion::Dot( const Quaternion& other ) const
	{ 
		return ( this->x * other.x + this->y * other.y + this->z * other.z + this->w * other.w );
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
			
	// Returns inverse of quaternion
	Quaternion Quaternion::Inverse( )
	{ 
		return Conjugate( ) / Dot( *this );
	}

	Quaternion Quaternion::Inverse(Quaternion& Q)
	{
		Quaternion C = Q.Conjugate() / Q.Dot(Q);
		return C;
	}

	f32 Quaternion::Angle() const
	{
		return 2.0f * std::acos(w);
	}

	Vec3 Quaternion::Axis() const
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
		return ( Vec3( this->Pitch( ), this->Yaw( ), this->Roll( ) ) * ( 180.0f / ( f32 )PI ) );
	} 

	Quaternion Quaternion::NegativeAngleAxis( ) const
	{
		return AngleAxis( -this->Angle( ), this->Axis( ) );
	}

	Vec3 Quaternion::operator*(const Vec3& V) const
	{
		//Vec3 Qxyz = Vec3(x, y, z); 
		//f32 s = this->w; 
		//Vec3 vprime = 2.0f * Qxyz.Dot( V ) * Qxyz + ( s * s - Qxyz.Dot( Qxyz ) ) * V + 2.0f * s * Qxyz.Cross( V );
		//return vprime; 

		return this->Rotate( V );
		
		//const Vec3 t = 2.0f * Qxyz.Cross( V );
		//return ( V + w * t + Qxyz.Cross( t ) ); 
	} 

	Quaternion Quaternion::Slerp( const Quaternion& a, const Quaternion& b, const f32& t )
	{
		f32 c = a.Dot( b );

		Quaternion end = b;
		if ( c < 0.0f )
		{
			// Reverse all signs
			c *= -1.0f;
			end.x *= -1.0f;
			end.y *= -1.0f;
			end.z *= -1.0f;
			end.w *= -1.0f;
		}

		// Calculate coefficients
		f32 sclp, sclq;
		if ( ( 1.0f - c ) > 0.0001f )
		{
			f32 omega = std::acosf( c );
			f32 s = std::sinf( omega );
			sclp = std::sinf( ( 1.0f - t ) * omega ) / s;
			sclq = std::sinf( t * omega ) / s; 
		}
		else
		{
			sclp = 1.0f - t;
			sclq = t;
		}

		Quaternion q;
		q.x = sclp * a.x + sclq * end.x;
		q.y = sclp * a.y + sclq * end.y;
		q.z = sclp * a.z + sclq * end.z;
		q.w = sclp * a.w + sclq * end.w;

		return q;
	}
			
	// Modified from: http://assimp.sourcearchive.com/documentation/2.0.863plus-pdfsg-1/aiQuaternion_8h_source.html
	Quaternion Quaternion::RotationBetweenVectors( const Vec3& s, const Vec3& d )
	{
		Vec3 start = start.Normalize( );
		Vec3 dest = d.Normalize( );

		f32 cosTheta = start.Dot( dest );
		Vec3 rotationAxis;

		if ( cosTheta < -1 + 0.001f ) {
			// special case when vectors in opposite directions :
			// there is no "ideal" rotation axis
			// So guess one; any will do as long as it's perpendicular to start
			// This implementation favors a rotation around the Up axis,
			// since it's often what you want to do.
			rotationAxis = Vec3( 0.0f, 0.0f, 1.0f ).Cross( start );
			if ( rotationAxis.Length2() < 0.01 ) // bad luck, they were parallel, try again!
				rotationAxis = Vec3( 1.0f, 0.0f, 0.0f ).Cross( start );

			rotationAxis = rotationAxis.Normalize();
			return Quaternion::AngleAxis( Math::ToRadians( 180.0f ), rotationAxis );
		}

		// Implementation from Stan Melax's Game Programming Gems 1 article
		rotationAxis = start.Cross( dest );

		f32 scalar = sqrt( ( 1 + cosTheta ) * 2 );
		f32 invs = 1.0f / scalar;

		return Quaternion(
			rotationAxis.x * invs,
			rotationAxis.y * invs,
			rotationAxis.z * invs,
			scalar * 0.5f
		);
	}

	Quaternion Quaternion::FromEulerAngles( const f32& yawDeg, const f32& pitchDeg, const f32& rollDeg )
	{
		f32 yaw = Math::ToRadians( rollDeg );
		f32 pitch = Math::ToRadians( pitchDeg );
		f32 roll = Math::ToRadians( yawDeg );

		Quaternion Q; 

		f32 cy = std::cosf( yaw * 0.5f );
		f32 sy = std::sinf( yaw * 0.5f );
		f32 cr = std::cosf( roll * 0.5f );
		f32 sr = std::sinf( roll * 0.5f );
		f32 cp = std::cosf( pitch * 0.5f );
		f32 sp = std::sinf( pitch * 0.5f );

		Q.w = cy * cr * cp + sy * sr * sp;
		Q.x = cy * sr * cp - sy * cr * sp;
		Q.y = cy * cr * sp + sy * sr * cp;
		Q.z = sy * cr * cp - cy * sr * sp;

		return Q;
	} 
}

























