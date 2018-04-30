#include "Math/Mat4.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/glm.hpp>

#include <sstream>

namespace Enjon 
{ 
	Mat4x4::Mat4x4()
	{
		//Set each element to 0.0f
		*this = Mat4x4::Identity();
	}

	Mat4x4::Mat4x4(const f32& diagonal)
	{
		for (int i = 0; i < 4 * 4; i++)
			elements[i] = 0.0f;

		//Set the actual diagonal elements
		elements[0 + 0 * 4] = diagonal;
		elements[1 + 1 * 4] = diagonal;
		elements[2 + 2 * 4] = diagonal;
		elements[3 + 3 * 4] = diagonal;

	}
			
	Mat4x4::Mat4x4(const Mat4x4& other)
	{ 
		for (u32 i = 0; i < 16; ++i)
		{
			elements[i] = other.elements[i];
		}
	}
		
	Mat4x4& Mat4x4::Multiply(const Mat4x4& other)
	{ 
		f32 data[16];
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				f32 sum = 0.0f;
				for (int e = 0; e < 4; e++)
				{
					sum += elements[x + e * 4] * other.elements[e + y * 4];
				}
				data[x + y * 4] = sum;
			}
		}
		memcpy(elements, data, 4 * 4 * sizeof(f32));

		return *this;
	}
	
	Vec3 Mat4x4::Multiply(const Vec3& other) const
	{
		return Vec3(
			columns[0].x * other.x + columns[1].x * other.y + columns[2].x * other.z + columns[3].x,
			columns[0].y * other.x + columns[1].y * other.y + columns[2].y * other.z + columns[3].y,
			columns[0].z * other.x + columns[1].z * other.y + columns[2].z * other.z + columns[3].z
		);
	}
	
	Vec4 Mat4x4::Multiply(const Vec4& other) const
	{
		return Vec4(
			columns[0].x * other.x + columns[1].x * other.y + columns[2].x * other.z + columns[3].x * other.w,
			columns[0].y * other.x + columns[1].y * other.y + columns[2].y * other.z + columns[3].y * other.w,
			columns[0].z * other.x + columns[1].z * other.y + columns[2].z * other.z + columns[3].z * other.w,
			columns[0].w * other.x + columns[1].w * other.y + columns[2].w * other.z + columns[3].w * other.w
		);
	}

	Mat4x4 operator*(Mat4x4 left, const Mat4x4& right)
	{
		return left.Multiply(right);
	}

	Mat4x4& Mat4x4::operator*=(const Mat4x4& other)
	{
		return Multiply(other);
	}
		

	Vec3 operator*(const Mat4x4& left, const Vec3& right)
	{
		return left.Multiply(right);
	}
		
	Vec4 operator*(const Mat4x4& left, const Vec4& right)
	{
		return left.Multiply(right);
	}

	Mat4x4 Mat4x4::Orthographic(const f32& left, 
							const f32& right, 
							const f32& bottom, 
							const f32& top, 
							const f32& near, 
							const f32& far)
	{
		//Create identiy matrix
		Mat4x4 result(1.0f);

		//Main diagonal
		result.elements[0 + 0 * 4] = 2.0f / (right - left);
		result.elements[1 + 1 * 4] = 2.0f / (top - bottom);
		result.elements[2 + 2 * 4] = 2.0f / (near - far);

		//Last column
		result.elements[0 + 3 * 4] = (left + right) / (left - right);
		result.elements[1 + 3 * 4] = (bottom + top) / (bottom - top);
		result.elements[2 + 3 * 4] = (far + near) / (far - near);

		return result; 
	}

	Mat4x4 Mat4x4::Perspective(const f32& FOV, 
							const f32& aspectRatio, 
							const f32& near, 
							const f32& far)
	{
		Mat4x4 result(0.0f);

		f32 q = 1.0f / tan(Math::ToRadians(0.5f * FOV));
		f32 a = q / aspectRatio;
		f32 b = (near + far) / (near - far);
		f32 c = (2.0f * near * far) / (near - far);

		result.elements[0 + 0 * 4] = a;		
		result.elements[1 + 1 * 4] = q;		
		result.elements[2 + 2 * 4] = b;		
		result.elements[2 + 3 * 4] = c;		
		result.elements[3 + 2 * 4] = -1.0f;		
		
		return result;
	}


	Mat4x4 Mat4x4::Identity()
	{
		return Mat4x4(1.0f);
	}

	Mat4x4 Mat4x4::Translate(const Vec3& vector)
	{
		//Identity matrix
		Mat4x4 result(1.0f);
		
		result.elements[0 + 3 * 4] = vector.x;
		result.elements[1 + 3 * 4] = vector.y;
		result.elements[2 + 3 * 4] = vector.z; 

		return result;
	}
	
	Mat4x4 Mat4x4::Scale(const Vec3& vector)
	{
		//Identity
		Mat4x4 result(1.0f);

		result.elements[0 + 0 * 4] = vector.x;
		result.elements[1 + 1 * 4] = vector.y;
		result.elements[2 + 2 * 4] = vector.z;

		return result;
	}

	Mat4x4 Mat4x4::Rotate(const f32& angle, const Vec3& axis)
	{
		//Identity
		Mat4x4 result(1.0f);
	
		f32 a = Math::ToRadians(angle);
		f32 c = cos(a);
		f32 s = sin(a);

		f32 x = axis.x;
		f32 y = axis.y;
		f32 z = axis.z;

		//First column
		result.elements[0 + 0 * 4] = x * x * (1 - c) + c;	
		result.elements[1 + 0 * 4] = x * y * (1 - c) + z * s;	
		result.elements[2 + 0 * 4] = x * z * (1 - c) - y * s;	
		
		//Second column
		result.elements[0 + 1 * 4] = x * y * (1 - c) - z * s;	
		result.elements[1 + 1 * 4] = y * y * (1 - c) + c;	
		result.elements[2 + 1 * 4] = y * z * (1 - c) + x * s;	
		
		//Third column
		result.elements[0 + 2 * 4] = x * z * (1 - c) + y * s;	
		result.elements[1 + 2 * 4] = y * z * (1 - c) - x * s;	
		result.elements[2 + 2 * 4] = z * z * (1 - c) + c;	

		return result; 
	}

	Mat4x4 Mat4x4::LookAt(const Vec3& Position, const Vec3& Target, const Vec3& Up)
	{
		Vec3 f = Vec3::Normalize( Target - Position );
		Vec3 s = Vec3::Normalize( f.Cross( Up ) );
		Vec3 u = s.Cross( f );
		
		Mat4x4 lookAt = Mat4x4::Identity( ); 
		lookAt[ 0 ][ 0 ] = s.x;
		lookAt[ 1 ][ 0 ] = s.y;
		lookAt[ 2 ][ 0 ] = s.z;

		lookAt[ 0 ][ 1 ] = u.x;
		lookAt[ 1 ][ 1 ] = u.y;
		lookAt[ 2 ][ 1 ] = u.z;

		lookAt[ 0 ][ 2 ] = -f.x;
		lookAt[ 1 ][ 2 ] = -f.y;
		lookAt[ 2 ][ 2 ] = -f.z;

		lookAt[ 3 ][ 0 ] = -s.Dot( Position );
		lookAt[ 3 ][ 1 ] = -u.Dot( Position );
		lookAt[ 3 ][ 2 ] = f.Dot( Position ); 

		return lookAt;
	}

	Mat4x4& Mat4x4::Invert()
	{
		double temp[16];

		temp[0] = elements[5] * elements[10] * elements[15] -
			elements[5] * elements[11] * elements[14] -
			elements[9] * elements[6] * elements[15] +
			elements[9] * elements[7] * elements[14] +
			elements[13] * elements[6] * elements[11] -
			elements[13] * elements[7] * elements[10];

		temp[4] = -elements[4] * elements[10] * elements[15] +
			elements[4] * elements[11] * elements[14] +
			elements[8] * elements[6] * elements[15] -
			elements[8] * elements[7] * elements[14] -
			elements[12] * elements[6] * elements[11] +
			elements[12] * elements[7] * elements[10];

		temp[8] = elements[4] * elements[9] * elements[15] -
			elements[4] * elements[11] * elements[13] -
			elements[8] * elements[5] * elements[15] +
			elements[8] * elements[7] * elements[13] +
			elements[12] * elements[5] * elements[11] -
			elements[12] * elements[7] * elements[9];

		temp[12] = -elements[4] * elements[9] * elements[14] +
			elements[4] * elements[10] * elements[13] +
			elements[8] * elements[5] * elements[14] -
			elements[8] * elements[6] * elements[13] -
			elements[12] * elements[5] * elements[10] +
			elements[12] * elements[6] * elements[9];

		temp[1] = -elements[1] * elements[10] * elements[15] +
			elements[1] * elements[11] * elements[14] +
			elements[9] * elements[2] * elements[15] -
			elements[9] * elements[3] * elements[14] -
			elements[13] * elements[2] * elements[11] +
			elements[13] * elements[3] * elements[10];

		temp[5] = elements[0] * elements[10] * elements[15] -
			elements[0] * elements[11] * elements[14] -
			elements[8] * elements[2] * elements[15] +
			elements[8] * elements[3] * elements[14] +
			elements[12] * elements[2] * elements[11] -
			elements[12] * elements[3] * elements[10];

		temp[9] = -elements[0] * elements[9] * elements[15] +
			elements[0] * elements[11] * elements[13] +
			elements[8] * elements[1] * elements[15] -
			elements[8] * elements[3] * elements[13] -
			elements[12] * elements[1] * elements[11] +
			elements[12] * elements[3] * elements[9];

		temp[13] = elements[0] * elements[9] * elements[14] -
			elements[0] * elements[10] * elements[13] -
			elements[8] * elements[1] * elements[14] +
			elements[8] * elements[2] * elements[13] +
			elements[12] * elements[1] * elements[10] -
			elements[12] * elements[2] * elements[9];

		temp[2] = elements[1] * elements[6] * elements[15] -
			elements[1] * elements[7] * elements[14] -
			elements[5] * elements[2] * elements[15] +
			elements[5] * elements[3] * elements[14] +
			elements[13] * elements[2] * elements[7] -
			elements[13] * elements[3] * elements[6];

		temp[6] = -elements[0] * elements[6] * elements[15] +
			elements[0] * elements[7] * elements[14] +
			elements[4] * elements[2] * elements[15] -
			elements[4] * elements[3] * elements[14] -
			elements[12] * elements[2] * elements[7] +
			elements[12] * elements[3] * elements[6];

		temp[10] = elements[0] * elements[5] * elements[15] -
			elements[0] * elements[7] * elements[13] -
			elements[4] * elements[1] * elements[15] +
			elements[4] * elements[3] * elements[13] +
			elements[12] * elements[1] * elements[7] -
			elements[12] * elements[3] * elements[5];

		temp[14] = -elements[0] * elements[5] * elements[14] +
			elements[0] * elements[6] * elements[13] +
			elements[4] * elements[1] * elements[14] -
			elements[4] * elements[2] * elements[13] -
			elements[12] * elements[1] * elements[6] +
			elements[12] * elements[2] * elements[5];

		temp[3] = -elements[1] * elements[6] * elements[11] +
			elements[1] * elements[7] * elements[10] +
			elements[5] * elements[2] * elements[11] -
			elements[5] * elements[3] * elements[10] -
			elements[9] * elements[2] * elements[7] +
			elements[9] * elements[3] * elements[6];

		temp[7] = elements[0] * elements[6] * elements[11] -
			elements[0] * elements[7] * elements[10] -
			elements[4] * elements[2] * elements[11] +
			elements[4] * elements[3] * elements[10] +
			elements[8] * elements[2] * elements[7] -
			elements[8] * elements[3] * elements[6];

		temp[11] = -elements[0] * elements[5] * elements[11] +
			elements[0] * elements[7] * elements[9] +
			elements[4] * elements[1] * elements[11] -
			elements[4] * elements[3] * elements[9] -
			elements[8] * elements[1] * elements[7] +
			elements[8] * elements[3] * elements[5];

		temp[15] = elements[0] * elements[5] * elements[10] -
			elements[0] * elements[6] * elements[9] -
			elements[4] * elements[1] * elements[10] +
			elements[4] * elements[2] * elements[9] +
			elements[8] * elements[1] * elements[6] -
			elements[8] * elements[2] * elements[5];

		double determinant = elements[0] * temp[0] + elements[1] * temp[4] + elements[2] * temp[8] + elements[3] * temp[12];
		determinant = 1.0 / determinant;

		for (int i = 0; i < 4 * 4; i++)
			elements[i] = temp[i] * determinant;

		return *this;
	}

	Mat4x4 Mat4x4::Inverse(const Mat4x4& M)
	{
		Mat4x4 R = M;
		return R.Invert();
	}

	std::ostream& operator<<(std::ostream& stream, const Mat4x4& Mat)
	{
		stream << "Mat4x4: \n\t";
		
		for(auto c : Mat.columns)
		{
			stream << c << "\n\t";
		}	

		return stream;
	}
}