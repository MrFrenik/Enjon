#ifndef ENJON_TRANSFORM_H
#define ENJON_TRANSFORM_H

#include <Defines.h>
#include <System/Types.h>
#include <Math/Common.h>
#include <Math/Quaternion.h>
#include <Math/Mat4.h>
#include <Math/Vec3.h>

namespace Enjon { namespace Math {

	struct Transform
	{
		// Default constructor
		Transform();
		Transform(EM::Vec3& _Position, EM::Quaternion& _Orientation, EM::Vec3& _Scale);
		~Transform();

		// Multiplication of this transform with another
		Transform operator*(Transform& Parent) const;	

		// Friend operator for dividing two transforms
		friend Transform operator/(Transform& World, Transform& Parent);

		// Gets inverse of this transform
		Transform Inverse();

		EM::Vec3 GetPosition()& { return Position; }
		EM::Vec3 GetScale()& { return Scale; }
		EM::Quaternion GetOrientation()& { return Orientation; }

		void SetPosition(EM::Vec3& position);
		void SetScale(EM::Vec3& scale);
		void SetOrientation(EM::Quaternion& orientation);

		// Member variables
		EM::Vec3 Position;
		EM::Quaternion Orientation;
		EM::Vec3 Scale;
	};


}}


#endif