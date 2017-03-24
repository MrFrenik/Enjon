/* ===========================================================
* @Author: 	       		John
* @Title:	       		camera3D.h 
* @Date:   	       		2015-11-08 11:31:34
* @Last Modified time: 	2015-11-20 16:23:59
=============================================================*/

#ifndef ENJON_CAMERA3D_H
#define ENJON_CAMERA3D_H

#include "Defines.h"
#include "System/Types.h"
#include "Math/Maths.h"


namespace Enjon {

	const float YAW 			= -90.0f;
	const float PITCH 			= 0.0f;
	const float SPEED 			= 0.05f;
	const float SENSITIVITY 	= 0.25f;
	const float ZOOM		 	= 45.0f;

	class Camera3D
	{
		public:
			Camera3D(EM::Vec3& Position, EM::Vec3 _Up = EM::Vec3(0.0f, 1.0f, 0.0f), float Yaw = YAW, float Pitch = PITCH)
				:
				Speed(SPEED), 
				Sensitivity(SENSITIVITY), 
				Zoom(ZOOM)
			{
				this->Position = Position;
				this->WorldUp = _Up;
				this->Yaw = Yaw;
				this->Front = EM::Vec3(0.0f, 0.0f, -1.0f); 

				// Update Camera3D's vectors
				this->Update();
			}

			~Camera3D(){}	

			// Responsible for updating all of the camera's vectors and view matrix
			void Update();
			// void Update(const EM::Vec2& MouseCoords, const EM::Vec2& ScreenDimensions);

			EM::Mat4 GetViewMatrix();

			void SetViewByMouse(const EM::Vec2& MouseCoords, const EM::Vec2& ScreenDimensions);
			void RotateCamera(float Angle, const EM::Vec3& Axis);

			void OffsetOrientation(const f32& Yaw, const f32& Pitch);

			EM::Vec3 Position;
			EM::Vec3 Front;
			EM::Vec3 Up; 
			EM::Vec3 WorldUp;
			EM::Vec3 Right;
			EM::Mat4 Orientation;
			float Yaw;
			float Pitch;
			float Speed;
			float Sensitivity;
			float Zoom;
	};
}

#endif