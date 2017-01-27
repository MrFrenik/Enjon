#include <Math/Transform.h>

namespace Enjon { namespace Math {

	Transform::Transform()
		: 
			Position(EM::Vec3(0.0f, 0.0f, 0.0f)), 
			Orientation(EM::Quaternion(0, 0, 0, 1)), 
			Scale(EM::Vec3(1, 1, 1))
	{
	}
		
	Transform::Transform(EM::Vec3& _Position, EM::Quaternion& _Orientation, EM::Vec3& _Scale)
		: 
			Position(_Position), 
			Orientation(_Orientation), 
			Scale(_Scale)
	{
	}

	Transform::~Transform()
	{
	}

	Transform Transform::operator*(Transform& Parent) const
	{
		Transform WorldSpace;

		WorldSpace.Position 	= Parent.Position + Parent.Orientation * (Parent.Scale * Position); 	
		WorldSpace.Orientation 	= Parent.Orientation * Orientation;
		WorldSpace.Scale 		= Parent.Scale * (Parent.Orientation * Scale);

		return WorldSpace;
	}

	Transform operator/(Transform& World, Transform& Parent)
	{
		Transform Local;

		auto ParentConjugate = Parent.Orientation.Conjugate();

		Local.Position 		= (ParentConjugate * (World.Position - Parent.Position)) / Parent.Scale;
		Local.Orientation 	= ParentConjugate * World.Orientation;
		Local.Scale 		= ParentConjugate * (World.Scale / Parent.Scale);

		return Local;
	}

	Transform Transform::Inverse()
	{
		Transform I;

		return I / *this;
	}

	void Transform::SetPosition(EM::Vec3& position)
	{
		Position = position;
	}

	void Transform::SetScale(EM::Vec3& scale)
	{
		Scale = scale;
	}

	void Transform::SetOrientation(EM::Quaternion& orientation)
	{
		Orientation = orientation;
	}

}}