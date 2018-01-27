#ifndef ENJON_CAMERA_H
#define ENJON_CAMERA_H

#include <Base/Object.h>
#include <Defines.h>
#include <System/Types.h>
#include <Math/Transform.h>
#include <Math/Vec3.h>
#include <Math/Vec2.h>
#include <Math/Mat4.h> 
#include <Math/Ray.h>

namespace Enjon 
{ 
	// TODO(): Move as many functions from this file to the source file

	ENJON_ENUM( )
	enum class ProjectionType
	{
		Perspective,
		Orthographic
	};

	ENJON_CLASS( Construct )
	class Camera : public Enjon::Object
	{
		ENJON_CLASS_BODY()

		public:
			/*
			* @brief Constructor
			*/
			Camera( );

			/*
			* @brief Constructor
			*/
			Camera(const u32& width, const u32& height);

			/*
			* @brief Constructor
			*/
			Camera(const iVec2& dimensions);

			/*
			* @brief Copy Constructor
			*/
			Camera(const Camera& Other);

			/*
			* @brief Destructor
			*/
			~Camera( ) = default;

			/*
			* @brief
			*/
			void LookAt(const Vec3& Position, const Vec3& Up = Vec3(0, 1, 0));

			/*
			* @brief
			*/
			void OffsetOrientation(const f32& Yaw, const f32& Pitch);

			/*
			* @brief
			*/
			Vec3 GetWorldPosition();

			/*
			* @brief
			*/
			Vec3 Forward() const;

			/*
			* @brief
			*/
			Vec3 Backward() const;

			/*
			* @brief
			*/
			Vec3 Right() const;

			/*
			* @brief
			*/
			Vec3 Left() const;

			/*
			* @brief
			*/
			Vec3 Up() const;

			/*
			* @brief
			*/
			Vec3 Down() const;

			/*
			* @brief
			*/
			Mat4 GetViewProjectionMatrix() const;

			/*
			* @brief
			*/
			Mat4 GetViewProjection() const;

			/*
			* @brief
			*/
			Mat4 GetProjection() const;

			/*
			* @brief
			*/
			Mat4 GetView() const;

			/*
			* @brief
			*/
			inline Vec2 GetNearFar() const 
			{ 
				return Vec2( NearPlane, FarPlane );
			}

			/*
			* @brief
			*/
			inline f32 GetNear() const
			{ 
				return NearPlane; 
			}

			/*
			* @brief
			*/
			inline f32 GetFar() const
			{ 
				return FarPlane; 
			}

			/*
			* @brief
			*/
			const ProjectionType GetProjectionType( ) const 
			{ 
				return ProjType;  
			}

			/*
			* @brief
			*/
			f32 GetOrthographicScale( ) const 
			{ 
				return OrthographicScale; 
			}

			/*
			* @brief
			*/
			f32 GetAspectRatio( ) const 
			{ 
				return ViewPortAspectRatio; 
			}

			/*
			* @brief
			*/
			inline void SetAspectRatio( const f32& aspectRatio ) 
			{ 
				ViewPortAspectRatio = aspectRatio; 
			}

			/*
			* @brief
			*/
			inline void SetNearFar(const f32& near, const f32& far) 
			{ 
				NearPlane = near; 
				FarPlane = far; 
			}

			/*
			* @brief
			*/
			inline void SetProjection(ProjectionType type) { ProjType = type; }

			/*
			* @brief
			*/
			inline void SetOrthographicScale(const f32& scale) 
			{ 
				OrthographicScale = scale; 
			}

			/*
			* @brief
			*/
			void SetProjectionType( ProjectionType type ) 
			{ 
				ProjType = type; 
			}

			/*
			* @brief
			*/
			void SetPosition(const Vec3& position);

			/*
			* @brief
			*/
			Vec3 GetPosition() const 
			{ 
				return Transform.GetPosition(); 
			}
			
			/*
			* @brief
			*/
			void SetRotation( const Quaternion& q );

			/*
			* @brief
			*/
			Quaternion GetRotation() const 
			{ 
				return Transform.Rotation; 
			}

			/*
			* @brief
			*/
			Ray ScreenToWorldRay( const f32& x, const f32& y );

			/*
			* @brief
			*/
			Ray ScreenToWorldRay( const Vec2& coords );

		private:

			/*
			* @brief
			*/
			Vec3 Unproject( const Vec3& screenCoords );

		public:

			// Member variables
			ENJON_PROPERTY()
			Transform Transform;

			ENJON_PROPERTY()
			f32 FOV	= 60.0f;

			ENJON_PROPERTY()
			f32 NearPlane = 0.1f;

			ENJON_PROPERTY()
			f32 FarPlane = 100.0f;

			ENJON_PROPERTY()
			f32 ViewPortAspectRatio;

			ENJON_PROPERTY()
			f32 OrthographicScale = 1.0f;

			ENJON_PROPERTY()
			ProjectionType ProjType	= ProjectionType::Perspective;

			Vec2 ScreenDimensions;
	};
}

#endif