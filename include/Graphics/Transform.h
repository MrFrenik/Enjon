#ifndef ENJON_TRANSFORM_H
#define ENJON_TRANSFORM_H

#include <Defines.h>
#include <System/Types.h>
#include <Math/Common.h>
#include <Math/Quaternion.h>
#include <Math/Mat4.h>
#include <Math/Vec3.h>

namespace Enjon { namespace Graphics {

	struct Transform
	{
		// Default constructor
		Transform()
			: 
				Position(EM::Vec3(0.0f, 0.0f, 0.0f)),
				Orientation(EM::Quaternion(0, 0, 0, 1)),
				Scale(EM::Vec3(1, 1, 1))
			{}

		// Multiplication of this transform with another
		Transform operator*(Transform& Parent) const;	

		// Friend operator for dividing two transforms
		friend Transform operator/(Transform& World, Transform& Parent);

		// Gets inverse of this transform
		Transform Inverse();

		// Member variables
		EM::Vec3 Position;
		EM::Quaternion Orientation;
		EM::Vec3 Scale;
	};


}}


#endif