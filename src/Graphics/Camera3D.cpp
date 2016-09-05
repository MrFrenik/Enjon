// /* ===========================================================
// * @Author: 	       		John
// * @Title:	       		camera3D.cpp 
// * @Date:   	       		2015-11-08 11:31:34
// * @Last Modified time: 	2016-01-06 20:46:46
// =============================================================*/

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

	EM::Mat4 Camera3D::GetViewMatrix()
	{
		return EM::Mat4::LookAt(this->Position, this->Position + this->Front, this->Up);
	}

}}









