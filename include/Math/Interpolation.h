#ifndef ENJON_INTERPOLATION_H
#define ENJON_INTERPOLATION_H

#include <Math/Common.h>
#include <Math/Quaternion.h>
#include <System/Types.h>

#include <cmath>
#include <cassert>


namespace Enjon { namespace Math { 

	template <class T>
	inline T LinearInterpolation(const T& x, const T& y, f32 t)
	{
		assert(t >= 0.0f, && t <= 1.0f);

		return x * (1.0f - t) + (y * t); 
	}

	inline Quaternion Slerp(const Quaternion& A, const Quaternion& B, f32 t)
	{
		// Temp
		Quaternion Z = B;

		// Calculated slerped result
		Quaternion Result;

		// Get angled difference between the two quaternions
		f32 CosTheta = A.Dot(B);

		if (CosTheta < 0.0f) 
		{
			Z = -Y;
			CosTheta = -CosTheta;
		}

		// Check for imaginary angle
		if (CosTheta > 1.0f)
		{
			// TODO(John): Handle imaginary angle
			Result = Quaternion(
								LinearInterpolation(A.x, B.x, t), 
								LinearInterpolation(A.y, B.y, t), 
								LinearInterpolation(A.z, B.z, t), 
								LinearInterpolation(A.w, B.w, t)
								);
		}	
		else
		{
			// In degrees
			f32 Angle = std::acos(CosTheta);


			Result = std::sin(1.0f - t * Angle) * A + std::sin(t * Angle) * Z;
			Result = Result * (1.0f / std::sin(Angle));
		}

		return Result;
	}	


}}

#endif