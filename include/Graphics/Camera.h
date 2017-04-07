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
		Perspective,
		Orthographic
	};

	struct Camera
	{
		Camera();
		Camera(u32 Width, u32 Height);
		Camera(iVec2& dimenstions);
		Camera(const Camera& Other) = default;

		void LookAt(Vec3& Position, Vec3& Up = Vec3(0, 1, 0));
		void OffsetOrientation(const f32& Yaw, const f32& Pitch);
		Vec3 GetWorldPosition();

		Vec3 Forward() const;
		Vec3 Backward() const;
		Vec3 Right() const;
		Vec3 Left() const;
		Vec3 Up() const;
		Vec3 Down() const;

		Mat4 GetViewProjectionMatrix() const;
		Mat4 GetViewProjection() const;
		Mat4 GetProjection() const;
		Mat4 GetView() const;
		inline Vec2 GetNearFar() const { return Vec2(NearPlane, FarPlane); }
		inline f32 GetNear() { return NearPlane; }
		inline f32 GetFar() { return FarPlane; }
		const ProjectionType GetProjectionType( ) const { return ProjType;  }
		f32 GetOrthographicScale( ) const { return OrthographicScale; }

		inline void SetNearFar(float Near, float Far) { NearPlane = Near; FarPlane = Far; }
		inline void SetProjection(ProjectionType Type) { ProjType = Type; }
		inline void SetOrthographicScale(const float Scale) { OrthographicScale = Scale; }
		void SetProjectionType( ProjectionType type ) { ProjType = type; }

		void SetPosition(Vec3& position);
		Vec3 GetPosition() { return Transform.GetPosition(); }
		
		Quaternion& GetRotation() { return Transform.Rotation; }

		// Member variables
		Transform Transform;
		f32 FOV								= 60.0f;
		f32 NearPlane						= 0.1f;
		f32 FarPlane						= 100.0f;
		f32 ViewPortAspectRatio;
		f32 OrthographicScale				= 1.0f;
		ProjectionType ProjType				= ProjectionType::Perspective;
		Vec2 ScreenDimensions;
	};

}

#endif