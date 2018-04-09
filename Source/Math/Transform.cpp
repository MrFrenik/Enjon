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

	Transform::Transform( const Transform& t )
		:
			mPosition( t.mPosition ),
			mRotation( t.mRotation ),
			mScale( t.mScale ),
			mEulerAngles( t.mEulerAngles )
	{
	}
	//==========================================================================
	
	Transform::Transform(const Vec3& position, const Quaternion& rotation, const Vec3& scale)
		: 
			mPosition(position), 
			mRotation(rotation), 
			mScale(scale)
	{
		mEulerAngles = mRotation.EulerAngles( );
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

	Vec3 Transform::GetEulerAngles( ) const
	{
		return mEulerAngles;
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
		// Set quaternion rotation
		mRotation = rotation;

		// Set euler angles from quaternion
		mEulerAngles = mRotation.EulerAngles( );
	} 

	//==========================================================================

	void Transform::SetRotation( const Vec3& eulerAngles )
	{
		// Set euler angles
		mEulerAngles = eulerAngles;

		// Set quaternion rotation from angles
		mRotation = Quaternion::FromEulerAngles( eulerAngles ); 
	}

	//==========================================================================

	void Transform::LookAt( const Vec3& target, const Vec3& up )
	{ 
		// Ignore, since you cannot look at yourself
		if ((mPosition - target).Length() < 0.001f) return;

		if (std::fabs(mPosition.Dot(up)) - 1.0f < 0.001f)
		{
			// Lookat and view direction are colinear
			return;
		}	

		// Get look at 
		Mat4 LA = Mat4::LookAt(mPosition, target, up);

		// Set rotation
		SetRotation( Mat4ToQuaternion(LA) ); 
	}

	//==========================================================================
}