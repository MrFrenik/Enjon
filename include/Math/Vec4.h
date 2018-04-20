#ifndef ENJON_VEC4_H
#define ENJON_VEC4_H

#include <iostream>
#include "Math/Vec2.h"
#include "Math/Vec3.h"

namespace Enjon {

	class Vec4
	{ 
		public: 

			/**
			* @brief Default constructor
			*/
			Vec4( )
				: x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f )
			{ 
			}

			/**
			* @brief Default d
			*/
			~Vec4( )
			{ 
			}

			/**
			* @brief Constructor that takes f32's for x, y, z, w
			* @param x - f32 value to be set for x component
			* @param y - f32 value to be set for y component
			* @param z - f32 value to be set for y component
			* @param w - f32 value to be set for y component
			*/
			Vec4(const f32& _x, const f32& _y, const f32& _z, const f32& _w)
				: x(_x), y(_y), z(_z), w(_w)
			{
			}

			/**
			* @brief Constructor that takes f32's for x, y, and vec2 for z, w
			* @param x - f32 value to be set for x component
			* @param y - f32 value to be set for y component
			* @param a - vec2 to be used to set z, w components
			*/
			Vec4(const f32& _x, const f32& _y, const Vec2& a)
				: x(_x), y(_y), z(a.x), w(a.y)
			{
			}

			/**
			* @brief Constructor that takes f32's for z, w, and vec2 for x, y
			* @param a - vec2 to be used to set x, y components
			* @param x - f32 value to be set for z component
			* @param y - f32 value to be set for w component
			*/
			Vec4(const Vec2& a, const f32& _z, const f32& _w)
				: x(a.x), y(a.y), z(_z), w(_w)
			{
			}

			/**
			* @brief Constructor that takes f32's for x, y, z, w
			* @param a - vec2 to be used to set x, y components
			* @param a - vec2 to be used to set z, w components
			*/
			Vec4(const Vec2& a, const Vec2& b)
				: x(a.x), y(a.y), z(b.x), w(b.y)
			{
			}

			/**
			* @brief Constructor that takes vec3 for x, y, z and f32 for w
			* @param a - vec3 to be used to set x, y, z components
			* @param a - f32 to be used to setw components
			*/
			Vec4(const Vec3& a, const f32& _w)
				: x(a.x), y(a.y), z(a.z), w(_w)
			{
			} 
			
			/**
			* @brief Constructor that takes f32's for x, y, z, w
			* @param val - f32 value to be set for all components

			*/
			Vec4(const f32& val)
				: x(val), y(val), z(val), w(val)
			{
			}
			
			friend std::ostream& operator<<(std::ostream& stream, Vec4& vector);	

			Vec4& Add(const Vec4& other);
			Vec4& Subtract(const Vec4& other);
			Vec4& Multiply(const Vec4& other);
			Vec4& Divide(const Vec4& other); 

			friend Vec4 operator+(Vec4 left, const Vec4& other);
			friend Vec4 operator-(Vec4 left, const Vec4& other); 
			friend Vec4 operator*(Vec4 left, const Vec4& other);
			friend Vec4 operator/(Vec4 left, const Vec4& other);

			friend Vec4 operator*(Vec4 left, const f32& scalar);
			friend Vec4 operator*(const f32& scalar, Vec4 right);
			friend void operator*=(Vec4& left, const f32& scalar);
			friend Vec4 operator*=(const f32& scalar, Vec4& left);
			friend Vec4 operator/(Vec4 left, const Vec4& other);
			friend Vec4 operator/(Vec4 left, f32 value);
			friend void operator/=(Vec4& left, f32 scalar);
			friend bool operator==(const Vec4& left, const Vec4& right); 
			friend bool operator!=(const Vec4& left, const Vec4& right);

			inline f32& operator[]( usize index ) { return data[ index ]; }
			inline const f32& operator[]( usize index ) const { return data[ index ]; }
			
			Vec4& operator+=(const Vec4& other);
			Vec4& operator-=(const Vec4& other);
			Vec4& operator*=(const Vec4& other);
			Vec4& operator/=(const Vec4& other);

			f32 Vec4::Length() const;
			f32 Vec4::Dot(const Vec4& other); 
			Vec3 XYZ(); 
			
			static Vec4 Normalize(const Vec4& vec);
			
		public:

			union 
			{
				struct 
				{
					f32 x; 
					f32 y;
					f32 z;
					f32 w; 
				};
				f32 data[ 4 ];

				struct
				{
					Vec2 xy;
					Vec2 zw;
				}; 

				Vec3 xyz;
			};
	};

	class iVec4
	{
		public:

			/**
			* @brief Default constructor
			*/
			iVec4() = default; 

			/**
			* @brief Constructor that takes s32's for x and y 		
			* @param x - s32 value to be set for x component
			* @param y - s32 value to be set for y component
			*/
			iVec4( const s32& _x, const s32& _y, const s32& _z, const s32& _w )
				: x( _x ), y( _y ), z( _z ), w( _w )
			{
			} 

			/**
			* @brief Constuctor that takes one s32 and assigns it to both x and y
			* @param val - s32 value to be set
			*/
			iVec4(const s32& val)
				: x(val), y(val), z(val), w(val)
			{
			}
			
			/**
			* @brief Default Destructor
			*/
			~iVec4() {}
			
			/**
			* @brief Stream operator for displaying components of vector
			* @param stream - ostream to be used
			* @param vector - vector to be streamed
			*/
			friend std::ostream& operator<<(std::ostream& stream, iVec4& vector);	

			/**
			* @brief Adds components of this vector to another
			* @param other - vector to be added to this 
			* @return this
			*/
			iVec4& Add(const iVec4& other);
			
			/**
			* @brief Subtracts components of other vector from this
			* @return this
			*/
			iVec4& Subtract(const iVec4& other);
			
			/**
			* @brief Multiplies components of other vector to this
			* @return this
			*/
			iVec4& Multiply(const iVec4& other);
			
			/**
			* @brief Divides components of this vector by other
			* @return this
			*/
			iVec4& Divide(const iVec4& other); 
			
			/**
			* @brief Scales this vector by a f32 scalar
			* @param other - vector to be used as divisor 
			* @return this
			*/
			iVec4& Scale(const s32& scalar);

			/**
			* @brief Adds one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec4 - The sum of the two vectors as new vector
			*/
			friend iVec4 operator+(iVec4 left, const iVec4& other);
			
			/**
			* @brief Subtracts one vector from another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec4 - The difference of the two vectors as new vector
			*/
			friend iVec4 operator-(iVec4 left, const iVec4& other); 
			
			/**
			* @brief Multiplies one vector to another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec4 - The product of the two vectors as new vector
			*/
			friend iVec4 operator*(iVec4 left, const iVec4& other);
			
			/**
			* @brief Divides one vector by another 
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return iVec4 - The quotient of the two vectors as new vector
			*/
			friend iVec4 operator/(iVec4 left, const iVec4& other);
			
			/**
			* @brief Equality operator to check whether two vectors have same components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if same, False if not
			* @return
			*/
			friend b8 operator==(iVec4 left, const iVec4& other);
			
			/**
			* @brief Inequality operator to check whether two vectors have different components
			* @param left - LH vector in operation
			* @param other - RH vector in operation
			* @return b8 - True if not same, False if same
			*/
			friend b8 operator!=(iVec4 left, const iVec4& other);
			
			/**
			* @brief Adds other vector's components onto this vector's components
			* @param other - vector to be added
			* @return this
			*/
			iVec4& operator+=(const iVec4& other);
			
			/**
			* @brief Subtracts other vector's components from this vector's components
			* @param other - vector to be subtracted
			* @return this
			*/
			iVec4& operator-=(const iVec4& other);
			
			/**
			* @brief Multiplies other vector's components by this vector's components
			* @param other - vector to be multiplied
			* @return this
			*/
			iVec4& operator*=(const iVec4& other);
			
			/**
			* @brief Divides other vector's components into this vector's components
			* @param other - vector to be divided by
			* @return this
			*/
			iVec4& operator/=(const iVec4& other); 
			
			/**
			* @brief Multiplies vector by f32 scalar quantity
			* @param scalar - quantity to multiply by
			* @return this
			*/
			iVec4& operator*=(const s32& scalar);
			
		public:
			s32 x; 
			s32 y; 
			s32 z;
			s32 w;
	};
}

typedef Enjon::Vec4 v4;

#endif