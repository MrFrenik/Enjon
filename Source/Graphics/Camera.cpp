#include <Graphics/Camera.h>
#include <Math/Quaternion.h>
#include <Math/Constants.h>
#include <Math/Vec2.h>


namespace Enjon 
{ 
	//=======================================================================================================

	Camera::Camera()
		: ViewPortAspectRatio(4.0f / 3.0f), ScreenDimensions(Vec2(800, 600))
	{
	}

	//=======================================================================================================

	Camera::Camera(const u32& Width, const u32& Height)
		: ViewPortAspectRatio((Enjon::f32)Width / (Enjon::f32)Height), ScreenDimensions(Vec2(Width, Height))
	{
	}

	//=======================================================================================================

	Camera::Camera(const iVec2& dimensions)
		: ViewPortAspectRatio((Enjon::f32)dimensions.x / (Enjon::f32)dimensions.y), ScreenDimensions(Vec2(dimensions.x, dimensions.y))
	{
	}

	//=======================================================================================================

	void Camera::SetPosition(const Vec3& position)
	{
		Transform.Position = position;
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

	//	Vec3 forwardVector = ( target - Transform.Position ).Normalize( );
	//	f32 dot = Vec3( 0.0f, 0.0f, -1.0f ).Dot( forwardVector );

	//	f32 rotationAngle = ( f32 )std::acos( dot );
	//	Vec3 rotationAxis = ( Vec3( 0.0f, 0.0f, 1.0f ).Cross( forwardVector ) ).Normalize( );
	//	Quaternion rotation = Quaternion::AngleAxis( rotationAngle, rotationAxis ).Normalize( );

	//	SetRotation( rotation );
	//}

	//=======================================================================================================

	void Camera::LookAt(const Vec3& Position, const Vec3& Up)
	{
		Vec3& Pos = Transform.Position;

		// Ignore, since you cannot look at yourself
		if ((Pos - Position).Length() < 0.001f) return;

		if (std::fabs(Pos.Dot(Up)) - 1.0f < 0.001f)
		{
			// Lookat and view direction are colinear
			return;
		}	

		// Get look at 
		Mat4 LA = Mat4::LookAt(Pos, Position, Up);

		// Set Transform
		Transform.Rotation = Enjon::Mat4ToQuaternion(LA);
	} 

	//=======================================================================================================

	void Camera::SetRotation( const Quaternion& q )
	{
		Transform.Rotation = q;
	}

	//=======================================================================================================

	void Camera::OffsetOrientation(const f32& Yaw, const f32& Pitch)
	{
		Quaternion X = Quaternion::AngleAxis(Yaw, 	Vec3(0, 1, 0)); 	// Absolute Up
		Quaternion Y = Quaternion::AngleAxis(Pitch, Right());			// Relative Right

		Transform.Rotation = X * Y * Transform.Rotation;
	}

	//=======================================================================================================

	Vec3 Camera::Forward() const
	{
		return Transform.Rotation * Vec3(0, 0, -1);
	}

	//=======================================================================================================

	Vec3 Camera::Backward() const
	{
		return Transform.Rotation * Vec3(0, 0, 1);
	}

	//=======================================================================================================

	Vec3 Camera::Right() const
	{
		return Transform.Rotation * Vec3(1, 0, 0);
	}

	//=======================================================================================================

	Vec3 Camera::Left() const
	{
		return Transform.Rotation * Vec3(-1, 0, 0);
	}

	//=======================================================================================================

	Vec3 Camera::Up() const
	{
		return Transform.Rotation * Vec3(0, 1, 0);
	}

	//=======================================================================================================

	Vec3 Camera::Down() const
	{
		return Transform.Rotation * Vec3(0, -1, 0);
	}

	//=======================================================================================================

	Mat4 Camera::GetViewProjectionMatrix() const
	{
		return GetProjection() * GetView();
	}

	//=======================================================================================================

	Mat4 Camera::GetViewProjection() const 
	{
		return GetProjection() * GetView();
	}

	//=======================================================================================================

	Mat4 Camera::GetProjection() const
	{
		Mat4 Projection;

		switch(ProjType)
		{
			case ProjectionType::Perspective:
			{
				Projection = Mat4::Perspective(FOV, ViewPortAspectRatio, NearPlane, FarPlane);
			} break;

			case ProjectionType::Orthographic:
			{
				f32 Distance = 0.5f * (FarPlane - NearPlane);
				Projection = Mat4::Orthographic(
														-OrthographicScale * ViewPortAspectRatio, 
														OrthographicScale * ViewPortAspectRatio, 
														-OrthographicScale, 
														OrthographicScale, 
														-Distance, 
														Distance	
													);

			} break;
		}

		return Projection;
	}

	//=======================================================================================================

	Mat4 Camera::GetView() const
	{ 
		//Mat4 scale = Mat4::Scale(Vec3(1.0f, 1.0f, 1.0f) / Transform.GetScale());
		//Mat4 rotation = QuaternionToMat4(Transform.Rotation);
		//Mat4 translate = Mat4::Translate(Transform.Position * -1.0f); 

		return Mat4::LookAt( Transform.Position, Transform.Position + Forward( ), Up( ) );

		//return (scale * rotation * translate);
	}
}






















