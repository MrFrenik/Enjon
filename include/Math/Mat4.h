#ifndef MAT4_H
#define MAT4_H

#include "Math/Maths.h"

namespace Enjon { namespace Math { 

	struct Mat4
	{
		union
		{
			float elements[4 * 4];
			Vec4 columns[4];
		};
	
		Mat4();
		Mat4(float value);
 
		Mat4& Multiply(const Mat4& other);

		friend Mat4 operator*(Mat4 left, const Mat4& right);
		Mat4& operator*=(const Mat4& other);
		
		Vec3 Multiply(const Vec3& other) const;
		friend Vec3 operator*(const Mat4& left, const Vec3& right);
		
		Vec4 Multiply(const Vec4& other) const;
		friend Vec4 operator*(const Mat4& left, const Vec4& right);
		
		static Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far);
		static Mat4 Perspective(float FOV, float aspectRatio, float near, float far);

		static Mat4 Identity();
		static Mat4 Scale(const Vec3& vector);
		static Mat4 Translate(const Vec3& vector);
		static Mat4 Rotate(float angle, const Vec3& axis);
	};



}}










#endif