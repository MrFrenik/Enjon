// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Transform.cpp

#include <Math/Transform.h> 

namespace Enjon 
{ 
	//==========================================================================

	Transform::Transform()
		: 
			mPosition(Vec3(0.0f, 0.0f, 0.0f)), 
			mRotation(Quaternion(0, 0, 0, 1)), 
			mScale(Vec3(1, 1, 1))
	{
	}
		
	//==========================================================================

	Transform::Transform(const Transform& t)
		: 
			mPosition(t.mPosition), 
			mRotation(t.mRotation), 
			mScale(t.mScale)
	{
	}
	//==========================================================================
	
	Transform::Transform(const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		: 
			mPosition(position), 
			mRotation(rotation), 
			mScale(scale)
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

		WorldSpace.mPosition	= mRotation.Rotate( rhs.mPosition ) + mPosition;
		WorldSpace.mRotation 	= mRotation * rhs.mRotation;
		WorldSpace.mScale 		= mScale * rhs.mScale;

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

		Vec3 inverseScale = 1.0f / rhs.mScale;
		Quaternion inverseRotation = rhs.mRotation.Inverse( );
		auto ParentConjugate = rhs.mRotation.Conjugate();

		Local.mPosition 	= (ParentConjugate * (mPosition - rhs.mPosition)) / rhs.mScale;
		Local.mRotation 	= ParentConjugate * mRotation;
		Local.mScale 		= (mScale / rhs.mScale);

		return Local;
	}

	//==========================================================================

	Vec3 Transform::GetPosition( ) const
	{
		return mPosition;
	}

	//==========================================================================
	
	Vec3 Transform::GetScale( ) const
	{
		return mScale;
	}

	//==========================================================================
	
	Quaternion Transform::GetRotation( ) const
	{
		return mRotation;
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
		mPosition = position;
	}

	//==========================================================================

	void Transform::SetScale(const Vec3& scale)
	{
		mScale = Vec3(scale);
	}

	//==========================================================================

	void Transform::SetScale(const f32& scale)
	{
		mScale = scale;
	}

	//==========================================================================
	
	void Transform::SetRotation(const Quaternion& rotation)
	{
		mRotation = rotation;
	} 
}