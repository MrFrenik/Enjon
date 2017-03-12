#include <Math/Transform.h>

namespace Enjon { namespace Math {

	//-----------------------------------------------------------------------
	Transform::Transform()
		: 
			Position(EM::Vec3(0.0f, 0.0f, 0.0f)), 
			Rotation(EM::Quaternion(0, 0, 0, 1)), 
			Scale(EM::Vec3(1, 1, 1))
	{
	}
		
	//-----------------------------------------------------------------------
	Transform::Transform(EM::Vec3& position, EM::Quaternion& rotation, EM::Vec3& scale)
		: 
			Position(position), 
			Rotation(rotation), 
			Scale(scale)
	{
	}

	//-----------------------------------------------------------------------
	Transform::~Transform()
	{
	}

	//-----------------------------------------------------------------------
	Transform Transform::operator*(Transform& Parent) const
	{
		Transform WorldSpace;

		WorldSpace.Position 	= Parent.Position + Parent.Rotation * (Parent.Scale * Position); 	
		WorldSpace.Rotation 	= Parent.Rotation * Rotation;
		WorldSpace.Scale 		= Parent.Scale * Scale;

		return WorldSpace;
	}

	Transform& Transform::operator*=(Transform& parent)
	{
		*this = *this * parent;
		return *this;	
	}	

	//-----------------------------------------------------------------------
	Transform operator/(Transform& World, Transform& Parent)
	{
		Transform Local;

		auto ParentConjugate = Parent.Rotation.Conjugate();

		Local.Position 		= (ParentConjugate * (World.Position - Parent.Position)) / Parent.Scale;
		Local.Rotation 		= ParentConjugate * World.Rotation;
		Local.Scale 		= ParentConjugate * (World.Scale / Parent.Scale);

		return Local;
	}

	//-----------------------------------------------------------------------
	Transform Transform::Inverse()
	{
		Transform I;

		return I / *this;
	}

	//-----------------------------------------------------------------------
	void Transform::SetPosition(EM::Vec3& position)
	{
		Position = position;
	}

	//-----------------------------------------------------------------------
	void Transform::SetScale(EM::Vec3& scale)
	{
		Scale = EM::Vec3(scale);
	}

	//-----------------------------------------------------------------------
	void Transform::SetScale(float scale)
	{
		Scale = scale;
	}

	//-----------------------------------------------------------------------
	void Transform::SetRotation(EM::Quaternion& rotation)
	{
		Rotation = rotation;
	}

}}