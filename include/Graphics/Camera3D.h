/* ===========================================================
* @Author: 	       		John
* @Title:	       		camera3D.h 
* @Date:   	       		2015-11-08 11:31:34
* @Last Modified time: 	2015-11-20 16:23:59
=============================================================*/

#ifndef ENJON_CAMERA3D_H
#define ENJON_CAMERA3D_H

#include "Defines.h"
#include "Math/Maths.h"

/* Defines possible options for camera movement */
enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

const float YAW 			= -90.0f;
const float PITCH 			= 0.0f;
const float SPEED 			= 3.0f;
const float SENSITIVITY 	= 0.25f;
const float ZOOM		 	= 45.0f;

namespace Enjon { namespace Graphics { namespace Camera3D {

	typedef struct Cam3D* Camera3D;

	extern inline struct Cam3D* New(int screenWidth, int screenHeight);
	extern inline EM(Mat4) GetViewMatrix(struct Cam3D* Camera);
	extern inline float GetZoom(struct Cam3D* Camera);
	extern inline void ProcessKeyboard(struct Cam3D* Camera, CameraMovement Direction);
	extern inline void ProcessMouseMovement(struct Cam3D* Camera, float XOffSet, float YOffSet, bool ConstrainPitch);

}}}

#endif