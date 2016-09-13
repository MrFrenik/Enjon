// /* ===========================================================
// * @Author: 	       		John
// * @Title:	       		camera3D.cpp 
// * @Date:   	       		2015-11-08 11:31:34
// * @Last Modified time: 	2016-01-06 20:46:46
// =============================================================*/

#include "SDL2/SDL.h"
#include "Defines.h"
#include "Graphics/Camera3D.h"

namespace Enjon { namespace Graphics { 


	void Camera3D::Update()
	{
		// Calculate front
		EM::Vec3 Front;
		Front.x = cos(EM::ToRadians(this->Yaw)) * cos(EM::ToRadians(this->Pitch));
		Front.y = sin(EM::ToRadians(this->Pitch));
		Front.z = sin(EM::ToRadians(this->Yaw)) * cos(EM::ToRadians(this->Pitch));

		// // Normalize and set front		
		this->Front = EM::Vec3::Normalize(Front);	

		// Calculate right and up
		this->Right = EM::Vec3::Normalize(this->Front.CrossProduct(this->WorldUp));
		this->Up  	= EM::Vec3::Normalize(this->Right.CrossProduct(this->Front));
	}

	// void Camera3D::Update(const EM::Vec2& MouseCoords, const EM::Vec2& ScreenDimensions)
	// {
	// 	SetViewByMouse(MouseCoords, ScreenDimensions);
	// }

	void Camera3D::OffsetOrientation(const f32& Yaw, const f32& Pitch)
	{
		
	}

	EM::Mat4 Camera3D::GetViewMatrix()
	{
		return EM::Mat4::LookAt(this->Position, this->Position + this->Front, this->Up);
	}

	void Camera3D::SetViewByMouse(const EM::Vec2& MouseCoords, const EM::Vec2& ScreenDimensions)
	{
		// Vector that describes MouseCoords - Center
		// EM::Vec3 MouseDirection(0.0f, 0.0f, 0.0f);

		// auto MiddleX = ScreenDimensions.x / 2.0f;
		// auto MiddleY = ScreenDimensions.y / 2.0f;

		// static float CurrentRotationAboutX = 0.0f;

		// float MaxAngle = 1.0f;

		// if ((MouseCoords.x == MiddleX) && (MouseCoords.y == MiddleY)) return;

		// MouseDirection.x = (MiddleX - MouseCoords.x) / Sensitivity;
		// MouseDirection.y = (MiddleY - MouseCoords.y) / Sensitivity;

		// CurrentRotationAboutX += MouseDirection.y;

		// if (CurrentRotationAboutX > 1)
		// {
		// 	CurrentRotationAboutX = 1;
		// 	return;
		// }

		// else if (CurrentRotationAboutX < -1)
		// {
		// 	CurrentRotationAboutX = -1;
		// 	return;
		// }
		// else
		// {
		// 	EM::Vec3 Axis = (Front - Position).CrossProduct(Up);
		// 	Axis = EM::Vec3::Normalize(Axis);

		// 	RotateCamera(MouseDirection.y, Axis);
		// 	RotateCamera(MouseDirection.x, EM::Vec3(0, 1, 0));
		// }
	}

	void Camera3D::RotateCamera(float Angle, const EM::Vec3& Axis)
	{
		// EM::Quaternion Temp, Quat_View(Front, 0), Result;

		// Temp.x = Axis.x * sin(Angle/2.0f);
		// Temp.y = Axis.y * sin(Angle/2.0f);
		// Temp.z = Axis.z * sin(Angle/2.0f);
		// Temp.w = cos(Angle/2.0f);

		// Result = (Temp * Quat_View) * Temp.Conjugate();

		// Front.x = Result.x;
		// Front.y = Result.y;
		// Front.z = Result.z;
	}
}}









