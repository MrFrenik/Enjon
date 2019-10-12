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

	class GraphicsScene;

	ENJON_ENUM( )
	enum class ProjectionType
	{
		Perspective,
		Orthographic
	};

	ENJON_CLASS( )
	class Camera : public Enjon::Object
	{
		ENJON_CLASS_BODY( Camera )

		public:
			/*
			* @brief Constructor
			*/
			virtual void ExplicitConstructor( ) override; 

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
			Mat4x4 GetViewProjectionMatrix() const;

			/*
			* @brief
			*/
			Mat4x4 GetViewProjection() const;

			/*
			* @brief
			*/
			Mat4x4 GetProjection() const;

			/*
			* @brief
			*/
			Mat4x4 GetPerspectiveProjection( ) const;

			/*
			* @brief
			*/
			Mat4x4 GetView() const;

			/*
			* @brief
			*/
			inline Vec2 GetNearFar() const 
			{ 
				return Vec2( mNearPlane, mFarPlane );
			}

			/*
			* @brief
			*/
			inline f32 GetNear() const
			{ 
				return mNearPlane; 
			}

			/*
			* @brief
			*/
			inline f32 GetFar() const
			{ 
				return mFarPlane; 
			}

			/*
			* @brief
			*/
			const ProjectionType GetProjectionType( ) const 
			{ 
				return mProjType;  
			}

			/*
			* @brief
			*/
			f32 GetOrthographicScale( ) const 
			{ 
				return mOrthographicScale; 
			}

			/*
			* @brief
			*/
			f32 GetAspectRatio( ) const 
			{ 
				return mViewPortAspectRatio; 
			}

			/*
			* @brief
			*/
			inline void SetAspectRatio( const f32& aspectRatio ) 
			{ 
				mViewPortAspectRatio = aspectRatio; 
			}

			/*
			* @brief
			*/
			inline void SetNearFar(const f32& n, const f32& f) 
			{ 
				mNearPlane = n; 
				mFarPlane = f; 
			}

			inline void SetFar( const f32& f )
			{
				mFarPlane = f; 
			}

			inline void SetNear( const f32& n )
			{
				mNearPlane = n;
			}

			/*
			* @brief
			*/
			inline void SetProjection(ProjectionType type) 
			{ 
				mProjType = type; 
			}

			/*
			* @brief
			*/
			inline void SetOrthographicScale(const f32& scale) 
			{ 
				mOrthographicScale = scale; 
			}

			/*
			* @brief
			*/
			void SetProjectionType( ProjectionType type ) 
			{ 
				mProjType = type; 
			} 

			Transform GetTransform() const
			{
				return mTransform;
			}

			/*
			* @brief
			*/
			Vec3 TransformPoint( const Vec3& point ) const;

			/*
			* @brief
			*/
			void SetTransform( const Transform& transform );

			/*
			* @brief
			*/
			void SetPosition(const Vec3& position);

			/*
			* @brief
			*/
			Vec3 GetPosition() const 
			{ 
				return mTransform.GetPosition(); 
			}

			/*
			* @brief
			*/
			void SetRotation( const Vec3& eulerAngles );
			
			/*
			* @brief
			*/
			void SetRotation( const Quaternion& q );

			/*
			* @brief
			*/
			Quaternion GetRotation() const 
			{ 
				return mTransform.GetRotation(); 
			}

			/*
			* @brief
			*/
			Ray ScreenToWorldRay( const f32& x, const f32& y );

			/*
			* @brief
			*/
			Ray ScreenToWorldRay( const Vec2& coords );

			/*
			* @brief
			*/
			void SetGraphicsScene( GraphicsScene* scene );

			/*
			* @brief
			*/
			GraphicsScene* GetGraphicsScene( ) const;

		private:

			/*
			* @brief
			*/
			Vec3 Unproject( const Vec3& screenCoords );

		private:

			// Member variables
			ENJON_PROPERTY()
			Transform mTransform;

			ENJON_PROPERTY()
			f32 mFOV = 60.0f;

			ENJON_PROPERTY()
			f32 mNearPlane = 0.1f;

			ENJON_PROPERTY()
			f32 mFarPlane = 1000.0f;

			ENJON_PROPERTY()
			f32 mViewPortAspectRatio;

			ENJON_PROPERTY()
			f32 mOrthographicScale = 1.0f;

			ENJON_PROPERTY()
			ProjectionType mProjType = ProjectionType::Perspective;

			Vec2 mScreenDimensions;

		private:

			GraphicsScene* mGraphicsScene = nullptr; 
	};
}

#endif