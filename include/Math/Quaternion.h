#ifndef ENJON_QUATERNION_H
#define ENJON_QUATERNION_H

#include <ostream>

#include <Defines.h>
#include <System/Types.h>
#include <Math/Vec3.h>
#include <Math/Common.h>
#include <Math/Mat4.h>

namespace Enjon { namespace Math {

	struct Quaternion
	{
		Quaternion() : x(0), y(0), z(0), w(1) {}

		Quaternion(const f32& X, const f32& Y, const f32& Z, const f32& W) : x(X), y(Y), z(Z), w(W) {}

		Quaternion(const EM::Vec3& N, f32 _w)
			: 
			x(N.x), 
			y(N.y), 
			z(N.z), 
			w(_w)
		{}

		Quaternion::Quaternion(Quaternion& Q)
			: 
			x(Q.x), 
			y(Q.y), 
			z(Q.z),
			w(Q.w)
		{}

		Quaternion::Quaternion(const Quaternion& Q)
			: 
			x(Q.x),
			y(Q.y),
			z(Q.z),
			w(Q.w)
		{}

		// Return vec3 of x,y,z components of quaternion
		Vec3 XYZ();

		// Negative unary operator overload
		Quaternion operator-() const;

		// Addition operator overload
		Quaternion operator+(const Quaternion& Other) const;

		// Subtraction operator overload
		Quaternion operator-(const Quaternion& Other) const;

		// Multiplication operator overload
		Quaternion operator*(const Quaternion& Other) const;

		// Multiplication operator with scalar
		Quaternion operator*(f32 V) const;

		// Division operator overload with f32
		Quaternion operator/(f32 V) const;

		// Equality operator
		bool operator==(const Quaternion& Other) const;

		// Inequality operator
		bool operator!=(const Quaternion& Other) const;

		// Operator stream overload
		friend std::ostream& operator<<(std::ostream& stream, const Quaternion& Q);

		// Gets length of quaternion
		f32 Length();

		// Normalizes length of quaternion to 1
		Quaternion Normalize();

		// Returns conjugate of quaternion
		Quaternion Conjugate() const;

		// Returns dot product with another quaternion
		f32 Dot(Quaternion& Q);

		// Returns cross product with another quaternion
		Quaternion Cross(const Quaternion& Q) const;

		// Returns inverse of quaternion
		Quaternion Inverse(Quaternion& Q);

		// Get angle from quaternion in radians
		f32 Angle();

		// Get axis from quaternion
		Vec3 Axis();

		// Get Euler roll from quaternion
		f32 Roll();

		// Get Euler pitch from quaternion
		f32 Pitch();

		// Get Euler yaw from quaternion
		f32 Yaw();

		// Get Euler Angles from quaternion
		EM::Vec3 QuaternionToEulerAngles();

		static inline Quaternion AngleAxis(const f32& Angle, const Vec3& Axis)
		{
			Quaternion Q;

			// Normalize axis first	
			Vec3 A = EM::Vec3::Normalize(Axis);

			// Get scalar
			const f32 S = std::sin(0.5f * Angle); 

			// Set Q
			Q.x = A.x * S;
			Q.y = A.y * S;
			Q.z = A.z * S;
			Q.w = std::cos(0.5f * Angle);

			return Q;
		}

		// Member variables
		f32 x, y, z, w;
	};

	inline Quaternion operator*(f32 V, const Quaternion& Q)
	{
		return Q * V;	
	}

	inline Mat4 QuaternionToMat4(Quaternion B)
	{
		Mat4 Mat = EM::Mat4::Identity();
		auto Q = B.Normalize();

		f32 xx = Q.x*Q.x;	
		f32 yy = Q.y*Q.y;	
		f32 zz = Q.z*Q.z;	
		f32 xy = Q.x*Q.y;
		f32 xz = Q.x*Q.z;
		f32 yz = Q.y*Q.z;
		f32 wx = Q.w*Q.x;
		f32 wy = Q.w*Q.y;
		f32 wz = Q.w*Q.z;

		Mat.elements[0 * 4 + 0] = 1.0f - 2.0f * (yy + zz);
		Mat.elements[1 * 4 + 0] = 2.0f * (xy + wz);
		Mat.elements[2 * 4 + 0] = 2.0f * (xz - wy);

		Mat.elements[0 * 4 + 1] = 2.0f * (xy - wz);
		Mat.elements[1 * 4 + 1] = 1.0f - 2.0f * (xx + zz);
		Mat.elements[2 * 4 + 1] = 2.0f * (yz + wx);

		Mat.elements[0 * 4 + 2] = 2.0f * (xz + wy);
		Mat.elements[1 * 4 + 2] = 2.0f * (yz - wx);
		Mat.elements[2 * 4 + 2] = 1.0f - 2.0f * (xx + yy);

		return Mat;
	}

	// NOTE(John): Assumes matrix is only rotational matrix and has no skew applied
	inline Quaternion Mat4ToQuaternion(const EM::Mat4& Mat)
	{
		Quaternion Q;
		auto& E = Mat.elements;

		f32 FourXSquaredMinusOne 	= E[0 * 4 + 0] - E[1 * 4 + 1] - E[2 * 4 + 2];
		f32 FourYSquaredMinusOne 	= E[1 * 4 + 1] - E[0 * 4 + 0] - E[2 * 4 + 2];
		f32 FourZSquaredMinuesOne 	= E[2 * 4 + 2] - E[0 * 4 + 0] - E[1 * 4 + 1];
		f32 FourWSquaredMinusOne 	= E[0 * 4 + 0] + E[1 * 4 + 1] + E[2 * 4 + 2];

		int32 BiggestIndex = 0;
		f32 FourBiggestSquaredMinusOne = FourWSquaredMinusOne;

		if (FourXSquaredMinusOne > FourBiggestSquaredMinusOne)
		{
			FourBiggestSquaredMinusOne 	= FourXSquaredMinusOne;
			BiggestIndex 				= 1;
		}
		if (FourYSquaredMinusOne > FourBiggestSquaredMinusOne)
		{
			FourBiggestSquaredMinusOne 	= FourYSquaredMinusOne;
			BiggestIndex 				= 2;
		}
		if (FourZSquaredMinuesOne > FourBiggestSquaredMinusOne)
		{
			FourBiggestSquaredMinusOne 	= FourZSquaredMinuesOne;
			BiggestIndex 				= 3;
		}


		f32 BiggestVal 	= std::sqrt(FourBiggestSquaredMinusOne + 1.0f) * 0.5f;
		f32 Mult 		= 0.25f / BiggestVal;

		switch(BiggestIndex)
		{
			case 0:
			{
				Q.w = BiggestVal;
				Q.x = (E[2 * 4 + 1] - E[1 * 4 + 2]) * Mult;
				Q.y = (E[0 * 4 + 2] - E[2 * 4 + 0]) * Mult;
				Q.z = (E[1 * 4 + 0] - E[0 * 4 + 1]) * Mult;
			}
			break;
			case 1:
			{
				Q.w = (E[2 * 4 + 1] - E[1 * 4 + 2]) * Mult;
				Q.x = BiggestVal;
				Q.y = (E[1 * 4 + 0] + E[0 * 4 + 1]) * Mult;
				Q.z = (E[0 * 4 + 2] + E[2 * 4 + 0]) * Mult;
			}
			break;
			case 2:
			{
				Q.w = (E[0 * 4 + 2] - E[2 * 4 + 0]) * Mult;
				Q.x = (E[1 * 4 + 0] + E[0 * 4 + 1]) * Mult;
				Q.y = BiggestVal;
				Q.z = (E[2 * 4 + 1] + E[1 * 4 + 2]) * Mult;
			}
			break;
			case 3:
			{
				Q.w = (E[1 * 4 + 0] - E[0 * 4 + 1]) * Mult;
				Q.x = (E[0 * 4 + 2] + E[2 * 4 + 0]) * Mult;
				Q.y = (E[2 * 4 + 1] + E[1 * 4 + 2]) * Mult;
				Q.z = BiggestVal;
			}
			break;
			default:
			{

			}
			break;

		}

		return Q;
	}

	inline Vec3 operator*(const Quaternion& Q, const Vec3& V)
	{
		auto Qxyz = EM::Vec3(Q.x, Q.y, Q.z);
		Vec3 T = 2.0f * Qxyz.Cross(V);
		return (V + Q.w * T + Qxyz.Cross(T));
	}




}}

#endif