#include <Graphics/Camera.h>
#include <Math/Quaternion.h>
#include <Math/Constants.h>
#include <Math/Vec2.h>


namespace Enjon { namespace Graphics {

	Camera::Camera()
		: 
		Transform(),
		FieldOfView(60),
		NearPlane(0.01f),
		FarPlane(100.0f),
		ViewPortAspectRatio(4.0f / 3.0f), 
		ProjType(ProjectionType::PERSPECTIVE),
		OrthographicScale(1.0f),
		ScreenDimensions(EM::Vec2(800, 600))
	{
	}

	Camera::Camera(Enjon::uint32 Width, Enjon::uint32 Height)
		: 
		Transform(),
		FieldOfView(50),
		NearPlane(0.1f),
		FarPlane(100.0f),
		ViewPortAspectRatio((Enjon::f32)Width / (Enjon::f32)Height), 
		ProjType(ProjectionType::PERSPECTIVE),
		OrthographicScale(1.0f),
		ScreenDimensions(EM::Vec2(Width, Height))
	{
	}

	Camera::Camera(EM::iVec2& dimensions)
		: 
		Transform(),
		FieldOfView(50),
		NearPlane(0.01f),
		FarPlane(100.0f),
		ViewPortAspectRatio((Enjon::f32)dimensions.x / (Enjon::f32)dimensions.y), 
		ProjType(ProjectionType::PERSPECTIVE),
		OrthographicScale(1.0f),
		ScreenDimensions(EM::Vec2(dimensions.x, dimensions.y))
	{
	}

	void Camera::SetPosition(EM::Vec3& position)
	{
		Transform.Position = position;
	}

	void Camera::LookAt(EM::Vec3& Position, EM::Vec3& Up)
	{
		EM::Vec3& Pos = Transform.Position;

		// Ignore, since you cannot look at yourself
		if ((Pos - Position).Length() < 0.001f) return;

		if (std::fabs(Pos.Dot(Up)) - 1.0f < 0.001f)
		{
			// Lookat and view direction are colinear
			return;
		}	

		// Get look at 
		EM::Mat4 LA = EM::Mat4::LookAt(Pos, Position, Up);

		// Set Transform
		Transform.Orientation = EM::Mat4ToQuaternion(LA);
	}

	void Camera::OffsetOrientation(const f32& Yaw, const f32& Pitch)
	{
		EM::Quaternion X = EM::Quaternion::AngleAxis(Yaw, 	EM::Vec3(0, 1, 0)); 	// Absolute Up
		EM::Quaternion Y = EM::Quaternion::AngleAxis(Pitch, Right());				// Relative Right

		Transform.Orientation = X * Y * Transform.Orientation;
	}

	EM::Vec3 Camera::Forward() const
	{
		return Transform.Orientation * EM::Vec3(0, 0, -1);
	}

	EM::Vec3 Camera::Backward() const
	{
		return Transform.Orientation * EM::Vec3(0, 0, 1);
	}

	EM::Vec3 Camera::Right() const
	{
		return Transform.Orientation * EM::Vec3(1, 0, 0);
	}

	EM::Vec3 Camera::Left() const
	{
		return Transform.Orientation * EM::Vec3(-1, 0, 0);
	}

	EM::Vec3 Camera::Up() const
	{
		return Transform.Orientation * EM::Vec3(0, 1, 0);
	}

	EM::Vec3 Camera::Down() const
	{
		return Transform.Orientation * EM::Vec3(0, -1, 0);
	}

	EM::Mat4 Camera::GetViewProjectionMatrix() const
	{
		return GetProjection() * GetView();
	}

	EM::Mat4 Camera::GetViewProjection() const 
	{
		return GetProjection() * GetView();
	}

	EM::Mat4 Camera::GetProjection() const
	{
		EM::Mat4 Projection;

		switch(ProjType)
		{
			case ProjectionType::PERSPECTIVE:
			{
				Projection = EM::Mat4::Perspective(FieldOfView, ViewPortAspectRatio, NearPlane, FarPlane);
			} break;

			case ProjectionType::ORTHOGRAPHIC:
			{
				Enjon::f32 Distance = 0.5f * (FarPlane - NearPlane);
				Projection = EM::Mat4::Orthographic(
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

	EM::Mat4 Camera::GetView() const
	{
		EM::Mat4 View;

		View =  EM::Mat4::Scale(EM::Vec3(1.0f, 1.0f, 1.0f) / Transform.Scale) * EM::QuaternionToMat4(Transform.Orientation) * EM::Mat4::Translate(Transform.Position * -1.0f);

		return View;
	}
}}






















