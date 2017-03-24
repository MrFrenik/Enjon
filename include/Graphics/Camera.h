#ifndef ENJON_CAMERA_H
#define ENJON_CAMERA_H

#include <Defines.h>
#include <System/Types.h>
#include <Math/Transform.h>
#include <Math/Vec3.h>
#include <Math/Vec2.h>
#include <Math/Mat4.h> 

namespace Enjon { 

	enum class ProjectionType
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	struct Camera
	{
		Camera();
		Camera(u32 Width, u32 Height);
		Camera(iVec2& dimenstions);
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
		EM::Mat4 GetViewProjection() const;
		EM::Mat4 GetProjection() const;
		EM::Mat4 GetView() const;
		inline Vec2 GetNearFar() const { return Vec2(NearPlane, FarPlane); }
		inline f32 GetNear() { return NearPlane; }
		inline f32 GetFar() { return FarPlane; }

		inline void SetNearFar(float Near, float Far) { NearPlane = Near; FarPlane = Far; }
		inline void SetProjection(ProjectionType Type) { ProjType = Type; }
		inline void SetOrthographicScale(const float Scale) { OrthographicScale = Scale; }

		void SetPosition(EM::Vec3& position);
		EM::Vec3 GetPosition() { return Transform.GetPosition(); }
		
		EM::Quaternion& GetRotation() { return Transform.Rotation; }

		// Member variables
		EM::Transform Transform;
		f32 FOV;
		f32 NearPlane;
		f32 FarPlane;
		f32 ViewPortAspectRatio;
		f32 OrthographicScale;
		ProjectionType ProjType;
		Vec2 ScreenDimensions;
	};

}

#endif