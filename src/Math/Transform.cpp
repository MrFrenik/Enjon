// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Transform.cpp

#include <Math/Transform.h> 

namespace Enjon 
{ 
	//==========================================================================

	Transform::Transform()
		: 
			Position(Vec3(0.0f, 0.0f, 0.0f)), 
			Rotation(Quaternion(0, 0, 0, 1)), 
			Scale(Vec3(1, 1, 1))
	{
	}
		
	//==========================================================================

	Transform::Transform(const Transform& t)
		: 
			Position(t.Position), 
			Rotation(t.Rotation), 
			Scale(t.Scale)
	{
	}
	//==========================================================================
	
	Transform::Transform(const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		: 
			Position(position), 
			Rotation(rotation), 
			Scale(scale)
	{
	}

	//==========================================================================

	Transform::~Transform()
	{
	}

	//==========================================================================
	
	Transform Transform::operator*(Transform& Parent) const
	{
		Transform WorldSpace;

		WorldSpace.Position 	= Parent.Position + Parent.Rotation * (Parent.Scale * Position); 	
		WorldSpace.Rotation 	= Parent.Rotation * Rotation;
		WorldSpace.Scale 		= Parent.Scale * Scale;

		return WorldSpace;
	}

	//==========================================================================

	Transform& Transform::operator*=(Transform& parent)
	{
		*this = *this * parent;
		return *this;	
	}	

	//==========================================================================

	Transform operator/(Transform& World, Transform& Parent)
	{
		Transform Local;

		auto ParentConjugate = Parent.Rotation.Conjugate();

		Local.Position 		= (ParentConjugate * (World.Position - Parent.Position)) / Parent.Scale;
		Local.Rotation 		= ParentConjugate * World.Rotation;
		Local.Scale 		= ParentConjugate * (World.Scale / Parent.Scale);

		return Local;
	}

	//==========================================================================

	Transform Transform::Inverse()
	{
		Transform I;

		return I / *this;
	}

	//==========================================================================

	void Transform::SetPosition(const Vec3& position)
	{
		Position = position;
	}

	//==========================================================================

	void Transform::SetScale(const Vec3& scale)
	{
		Scale = Vec3(scale);
	}

	//==========================================================================

	void Transform::SetScale(const f32& scale)
	{
		Scale = scale;
	}

	//==========================================================================
	
	void Transform::SetRotation(const Quaternion& rotation)
	{
		Rotation = rotation;
	} 
}