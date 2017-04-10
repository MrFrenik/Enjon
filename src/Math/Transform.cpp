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
	
	Transform Transform::operator*(const Transform& rhs) const
	{
		Transform WorldSpace;

		WorldSpace.Position		= Rotation.Rotate( rhs.Position ) + Position;
		WorldSpace.Rotation 	= Rotation * rhs.Rotation;
		WorldSpace.Scale 		= Scale * rhs.Scale;

		return WorldSpace;
	}

	//==========================================================================

	Transform& Transform::operator*=(const Transform& parent)
	{
		*this = *this * parent;
		return *this;	
	}	

	//==========================================================================

	Transform Transform::operator/(Transform& rhs)
	{
		Transform Local;

		Vec3 inverseScale = 1.0f / rhs.Scale;
		Quaternion inverseRotation = rhs.Rotation.Inverse( );
		auto ParentConjugate = rhs.Rotation.Conjugate();

		Local.Position 		= (ParentConjugate * (Position - rhs.Position)) / rhs.Scale;
		Local.Rotation 		= ParentConjugate * Rotation;
		Local.Scale 		= (Scale / rhs.Scale);

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