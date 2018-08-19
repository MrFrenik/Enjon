// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Transform.cpp

#include <Math/Transform.h> 

namespace Enjon 
{ 
	//==========================================================================

	void Transform::ExplicitConstructor()
	{
		mPosition = Vec3( 0.0f, 0.0f, 0.0f );
		mRotation = Quaternion( 0, 0, 0, 1 );
		mScale = Vec3( 1, 1, 1 );
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

	// WorldScale	= ParentScale * LocalScale
	// WorldRot		= LocalRot * ParentRot
	// WorldTrans	= ParentPos + [ ParentRot * ( ParentScale * LocalPos ) ]
	Transform Transform::operator*( const Transform& rhs ) const
	{ 
		// Normalized rotations
		Quaternion rhsRotNorm = rhs.GetRotation( ).Normalize( );
		Quaternion rotNorm = GetRotation( ).Normalize( );

		// Calculate scale
		Vec3 scale = rhs.GetScale( ) * GetScale( );
		// Calculate rotation
		Quaternion rot = ( rhsRotNorm * rotNorm ).Normalize( );
		// Calculate translation
		Vec3 trans = rhs.GetPosition( ) + ( rhsRotNorm * ( rhs.GetScale( ) * GetPosition( ) ) );

		// Return final transformation
		return Transform( trans, rot, scale );
	} 

	//==========================================================================

	Transform& Transform::operator*=(const Transform& parent)
	{
		*this = *this * parent;
		return *this;	
	}	

	//========================================================================== 

	// RelScale = WorldScale / ParentScale 
	// RelRot	= Inverse(ParentRot) * WorldRot
	// RelTrans	= [Inverse(ParentRot) * (WorldPos - ParentPosition)] / ParentScale;
	Transform Transform::operator/( const Transform& rhs ) const
	{
		// Get inverse rotation normalized
		Quaternion parentInverse = rhs.GetRotation( ).Inverse( ).Normalize( );
		// Get rotation normalized
		Quaternion rotNorm = GetRotation( ).Normalize( ); 

		Vec3 relativeScale = GetScale( ) / rhs.GetScale( );
		Quaternion relativeRot = ( parentInverse * rotNorm ).Normalize( );
		Vec3 relativePos = ( parentInverse * ( GetPosition( ) - rhs.GetPosition( ) ) ) / rhs.GetScale( );

		// Return relative transform
		return Transform( relativePos, relativeRot, relativeScale ); 
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
		Mat4x4 LA = Mat4x4::LookAt(mPosition, target, up);

		// Set rotation
		SetRotation( Mat4x4ToQuaternion(LA) ); 
	}

	//==========================================================================

	Mat4x4 Transform::ToMat4x4( ) const
	{
		Mat4x4 mat = Mat4x4::Identity( );
		mat *= Mat4x4::Translate( mPosition );
		mat *= QuaternionToMat4x4( mRotation );
		mat *= Mat4x4::Scale( mScale ); 
		return mat;
	}

	//==========================================================================
}