#include "Graphics/Camera.h"
#include "Math/Quaternion.h"
#include "Math/Constants.h"
#include "Math/Vec2.h"
#include "Math/Transform.h"
#include "Engine.h"
#include "SubsystemCatalog.h"
#include "Graphics/GraphicsSubsystem.h"


namespace Enjon 
{ 
	//=======================================================================================================

	Camera::Camera()
		: mViewPortAspectRatio(4.0f / 3.0f), mScreenDimensions(Vec2(800, 600))
	{
	}

	//=======================================================================================================

	Camera::Camera(const u32& Width, const u32& Height)
		: mViewPortAspectRatio((Enjon::f32)Width / (Enjon::f32)Height), mScreenDimensions(Vec2(Width, Height))
	{
	}

	//=======================================================================================================

	Camera::Camera(const iVec2& dimensions)
		: mViewPortAspectRatio((Enjon::f32)dimensions.x / (Enjon::f32)dimensions.y), mScreenDimensions(Vec2(dimensions.x, dimensions.y))
	{
	}

	//=======================================================================================================

	Camera::Camera( const Camera& Other )
		: mViewPortAspectRatio( Other.mViewPortAspectRatio ), mScreenDimensions( Other.mScreenDimensions ), mTransform( Other.mTransform )
	{ 
	}

	//=======================================================================================================

	void Camera::SetPosition(const Vec3& position)
	{
		mTransform.SetPosition( position );
	}

	//=======================================================================================================

	void Camera::SetTransform( const Transform& transform )
	{
		mTransform = transform;
	}

	//=======================================================================================================

	//void Camera::LookAt( const Vec3& target, const Vec3& up )
	//{
	//	// Get normalized direction towards target
	//	//Vec3 direction = ( target - GetPosition( ) ).Normalize( );

	//	//// Find rotation between camera forward and desired target
	//	//Quaternion rot1 = Quaternion::RotationBetweenVectors( Forward( ), direction );

	//	//// Recompute desired up 
	//	//Vec3 right = direction.Cross( up );
	//	//Vec3 desiredUp = right.Cross( direction );

	//	//Vec3 newUp = rot1 * Vec3( 0.0f, 1.0f, 0.0f );
	//	//Quaternion rot2 = Quaternion::RotationBetweenVectors( newUp, up );

	//	//// Set orientation of camera
	//	//SetRotation( rot2 * rot1 );

	//	Vec3 forwardVector = ( target - mTransform.Position ).Normalize( );
	//	f32 dot = Vec3( 0.0f, 0.0f, -1.0f ).Dot( forwardVector );

	//	f32 rotationAngle = ( f32 )std::acos( dot );
	//	Vec3 rotationAxis = ( Vec3( 0.0f, 0.0f, 1.0f ).Cross( forwardVector ) ).Normalize( );
	//	Quaternion rotation = Quaternion::AngleAxis( rotationAngle, rotationAxis ).Normalize( );

	//	SetRotation( rotation );
	//}

	//=======================================================================================================

	void Camera::LookAt(const Vec3& Position, const Vec3& Up)
	{
		Vec3& Pos = mTransform.GetPosition();

		// Ignore, since you cannot look at yourself
		if ((Pos - Position).Length() < 0.001f) return;

		if (std::fabs(Pos.Dot(Up)) - 1.0f < 0.001f)
		{
			// Lookat and view direction are colinear
			return;
		}	

		// Get look at 
		Mat4x4 LA = Mat4x4::LookAt(Pos, Position, Up);

		// Set Transform
		mTransform.SetRotation( Mat4x4ToQuaternion(LA) );
	} 

	//=======================================================================================================

	void Camera::SetRotation( const Quaternion& q )
	{
		mTransform.SetRotation( q );
	}

	//=======================================================================================================

	void Camera::OffsetOrientation(const f32& Yaw, const f32& Pitch)
	{
		Quaternion X = Quaternion::AngleAxis(Yaw, 	Vec3(0, 1, 0)); 	// Absolute Up
		Quaternion Y = Quaternion::AngleAxis(Pitch, Right());			// Relative Right

		mTransform.SetRotation( X * Y * mTransform.GetRotation() );
	}

	//=======================================================================================================

	Vec3 Camera::Forward() const
	{
		return mTransform.GetRotation() * Vec3(0, 0, -1);
	}

	//=======================================================================================================

	Vec3 Camera::Backward() const
	{
		return mTransform.GetRotation() * Vec3(0, 0, 1);
	}

	//=======================================================================================================

	Vec3 Camera::Right() const
	{
		return mTransform.GetRotation() * Vec3(1, 0, 0);
	}

	//=======================================================================================================

	Vec3 Camera::Left() const
	{
		return mTransform.GetRotation() * Vec3(-1, 0, 0);
	}

	//=======================================================================================================

	Vec3 Camera::Up() const
	{
		return mTransform.GetRotation() * Vec3(0, 1, 0);
	}

	//=======================================================================================================

	Vec3 Camera::Down() const
	{
		return mTransform.GetRotation() * Vec3(0, -1, 0);
	}

	//=======================================================================================================

	Mat4x4 Camera::GetViewProjectionMatrix() const
	{
		return GetProjection() * GetView();
	}

	//=======================================================================================================

	Mat4x4 Camera::GetViewProjection() const 
	{
		return GetProjection() * GetView();
	}

	//=======================================================================================================

	Mat4x4 Camera::GetProjection() const
	{
		Mat4x4 projection;

		switch( mProjType )
		{
			case ProjectionType::Perspective:
			{
				projection = Mat4x4::Perspective( mFOV, mViewPortAspectRatio, mNearPlane, mFarPlane );
			} break;

			case ProjectionType::Orthographic:
			{
				f32 distance = 0.5f * ( mFarPlane - mNearPlane );
				projection = Mat4x4::Orthographic(
														-mOrthographicScale * mViewPortAspectRatio, 
														mOrthographicScale * mViewPortAspectRatio, 
														-mOrthographicScale, 
														mOrthographicScale, 
														-distance, 
														distance	
													);

			} break;
		}

		return projection;
	}

	//=======================================================================================================

	Mat4x4 Camera::GetView() const
	{ 
		//Mat4x4 scale = Mat4x4::Scale(Vec3(1.0f, 1.0f, 1.0f) / Transform.GetScale());
		//Mat4x4 rotation = QuaternionToMat4x4(Transform.Rotation);
		//Mat4x4 translate = Mat4x4::Translate(Transform.Position * -1.0f); 

		return Mat4x4::LookAt( mTransform.GetPosition(), mTransform.GetPosition() + Forward( ), Up( ) );

		//return (scale * rotation * translate);
	}

	//=======================================================================================================

	// Edited from Source Implementation: https://stackoverflow.com/questions/23644470/how-to-convert-mouse-coordinate-on-screen-to-3d-coordinate
	Vec3 Camera::Unproject( const Vec3& screenCoords )
	{
		Vec3 worldCoordinates;

		// Get inverse of view project matrix from camera
		Mat4x4 inverseViewProjection = Mat4x4::Inverse( GetViewProjection( ) );

		// Get viewport dimensions
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		iVec2 viewport = gfx->GetViewport( );

		// Screen coordinates to be used
		f32 winX = (f32)screenCoords.x;
		f32 winY = (f32)screenCoords.y;
		f32 winZ = (f32)screenCoords.z;

		// Transform from ndc
		Vec4 in;
		in.x = ( winX / (f32)viewport.x ) * 2.0f - 1.0f;
		in.y = 1.0f - ( winY / (f32)viewport.y ) * 2.0f;
		in.z = 2.0f * winZ - 1.0f;
		in.w = 1.0f; 

		// To world coordinates
		Vec4 out = inverseViewProjection * in;
		// Avoid division by zero
		if ( out.w == 0.0f )
		{
			return worldCoordinates; 
		}

		// W division to normalize
		out.w = 1.0f / out.w;
		worldCoordinates.x = out.x * out.w;
		worldCoordinates.y = out.y * out.w;
		worldCoordinates.z = out.z * out.w; 

		return worldCoordinates;
	}

	//=======================================================================================================

	Ray Camera::ScreenToWorldRay( const f32& x, const f32& y )
	{
		// Get start and end positions from near and far planes unprojected
		Vec3 start = Unproject( Vec3( x, y, 0.0f ) );
		Vec3 end = Unproject( Vec3( x, y, 1.0f ) );

		// Calculate normalized direction for ray
		Vec3 dir = Vec3::Normalize( end - start );

		Ray ray;
		ray.mPoint = start;
		ray.mDirection = dir;

		return ray; 
	}

	//=======================================================================================================

	Ray Camera::ScreenToWorldRay( const Vec2& coords )
	{
		return this->ScreenToWorldRay( coords.x, coords.y );
	}

	//=======================================================================================================

	void Camera::SetGraphicsScene( GraphicsScene* scene )
	{
		mGraphicsScene = scene;
	}

	//=======================================================================================================

	GraphicsScene* Camera::GetGraphicsScene( ) const
	{
		return mGraphicsScene;
	}

	//=======================================================================================================
}






















