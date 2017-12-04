#ifndef ENJON_MAT4_H
#define ENJON_MAT4_H

#include <ostream>
#include <Math/Vec3.h>
#include <Math/Vec2.h>
#include <Math/Vec4.h>
#include <Math/Common.h>

namespace Enjon {

	class Mat4
	{
		public:

			union
			{
				f32 elements[4 * 4];
				f32 Elements[4][4];
				struct  
				{
					Vec4 x, y, z, w;
				};
				Vec4 columns[4];
			};

			Mat4();

			Mat4(const f32& value);
			
			Mat4(const Mat4& other);

			Mat4& Multiply(const Mat4& other);

			~Mat4( )
			{ 
			}

			friend Mat4 operator*(Mat4 left, const Mat4& right);
			Mat4& operator*=(const Mat4& other);

			Vec3 Multiply(const Vec3& other) const;
			friend Vec3 operator*(const Mat4& left, const Vec3& right);

			Vec4 Multiply(const Vec4& other) const;
			friend Vec4 operator*(const Mat4& left, const Vec4& right);

			inline Vec4& operator[]( usize index ) { return columns[ index ]; }
			inline const Vec4& operator[]( usize index ) const { return columns[ index ]; }

			Mat4& Invert();

			static Mat4 Orthographic(const f32& left, 
									const f32& right, 
									const f32& bottom, 
									const f32& top, 
									const f32& near, 
									const f32& far);

			/*
			* @brief
			* @param FOV - field of view given in degrees
			* @param aspectRatio - aspectRatio of screen
			* @param near - near plane
			* @param far - far plane
			*/
			static Mat4 Perspective(const f32& FOV, const f32& aspectRatio, const f32& near, const f32& far);

			static Mat4 Identity();
			static Mat4 Scale(const Vec3& vector);
			static Mat4 Translate(const Vec3& vector);
			static Mat4 Rotate(const f32& angle, const Vec3& axis);
			static Mat4 Inverse(const Mat4& M);

			/*
			* @brief Look at RH
			*/
			static Mat4 LookAt(const Vec3& Position, const Vec3& Target, const Vec3& Up);

			friend std::ostream& operator<<(std::ostream& stream, const Mat4& Mat);
	};

}
typedef Enjon::Mat4 mat4;


#endif