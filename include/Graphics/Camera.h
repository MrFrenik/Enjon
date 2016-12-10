#ifndef ENJON_CAMERA_H
#define ENJON_CAMERA_H

#include <Defines.h>
#include <System/Types.h>
#include <Math/Transform.h>
#include <Math/Vec3.h>
#include <Math/Vec2.h>
#include <Math/Mat4.h>

namespace Enjon { namespace Graphics { 

	enum class ProjectionType
	{
		Perspective,
		Orthographic
	};

	struct Camera
	{
		Camera();
		Camera(Enjon::uint32 Width, Enjon::uint32 Height);
		Camera(const Camera& Other) = default;

		void LookAt(EM::Vec3& Position, EM::Vec3& Up = EM::Vec3(0, 1, 0));
		void OffsetOrientation(const f32& Yaw, const f32& Pitch);
		EM::Vec3 GetWorldPosition();

		EM::Vec3 Forward() const;
		EM::Vec3 Backward() const;
		EM::Vec3 Right() const;
		EM::Vec3 Left() const;
		EM::Vec3 Up() const;
		EM::Vec3 Down() const;

		EM::Mat4 GetViewProjectionMatrix() const;
		EM::Mat4 GetProjection() const;
		EM::Mat4 GetView() const;

		inline void SetNearFar(float Near, float Far) { NearPlane = Near; FarPlane = Far; }
		inline void SetProjection(ProjectionType Type) { ProjType = Type; }

		// Member variables
		EM::Transform Transform;
		Enjon::f32 FieldOfView;
		Enjon::f32 NearPlane;
		Enjon::f32 FarPlane;
		Enjon::f32 ViewPortAspectRatio;
		Enjon::f32 OrthographicScale;
		ProjectionType ProjType;
		EM::Vec2 ScreenDimensions;
	};

}}

#endif