// /* ===========================================================
// * @Author: 	       		John
// * @Title:	       		camera3D.cpp 
// * @Date:   	       		2015-11-08 11:31:34
// * @Last Modified time: 	2016-01-06 20:46:46
// =============================================================*/

// #include <stdlib.h>
// #include <assert.h>
// #include "Defines.h"
// #include "Graphics/Camera3D.h"

// namespace Enjon { namespace Graphics { namespace Camera3D {

// 	struct Cam3D 
// 	{
// 		int ScreenWidth;
// 		int ScreenHeight;
// 		int NeedsMatrixUpdate;
// 		float FOV;
// 		float Yaw;
// 		float Pitch;
// 		float MovementSpeed;
// 		float MouseSensitivity;
// 		float Zoom;
// 		EM(Vec3) Position;
// 		EM(Vec3) Front;
// 		EM(Vec3) Up;
// 		EM(Vec3) Right;
// 		EM(Vec3) WorldUp;
// 	};

// 	inline void UpdateCameraVectors(struct Cam3D* Camera);

// 	inline struct Cam3D* New(int screenWidth, int screenHeight)
// 	{
// 		struct Cam3D* cam = (struct Cam3D*)malloc(sizeof *cam);

// 		assert(cam);

// 		// Set up properties
// 		cam->ScreenWidth 	= screenWidth;
// 		cam->ScreenHeight 	= screenHeight;
// 		cam->FOV 			= 45.0f;
// 		cam->Yaw 			= YAW;
// 		cam->Pitch 			= PITCH;
// 		cam->Zoom 			= 45.0f;

// 		cam->Position 	= EM(Vec3)(0.0f, 0.0f, 0.0f);
// 		cam->Front 		= EM(Vec3)(0.0f, 0.0f, 0.0f);
// 		cam->Up 		= EM(Vec3)(0.0f, 1.0f, 0.0f);
// 		cam->Right 		= EM(Vec3)(0.0f, 0.0f, 0.0f);
// 		cam->WorldUp 	= EM(Vec3)(0.0f, 0.0f, 0.0f);

// 		cam->NeedsMatrixUpdate 	= 0;
// 		cam->MouseSensitivity 	= 1.0f;
// 		cam->MovementSpeed 		= 1.0f;

// 		UpdateCameraVectors(cam);

// 		return cam;
// 	}

// 	inline void UpdateCameraVectors(struct Cam3D* Camera)
// 	{
// 		assert(Camera);

// 		// Calculate new Front Vector
// 		EM(Vec3) Front;
// 		float Yaw = Camera->Yaw, Pitch = Camera->Pitch;
// 		Front.x = cos(EM(ToRadians(Yaw))) * cos(EM(ToRadians(Pitch)));
// 		Front.y = sin(EM(ToRadians(Pitch)));
// 		Front.z = sin(EM(ToRadians(Yaw))) * cos(EM(ToRadians(Pitch)));

// 		// Normalize Front
// 		EM(Vector)::Normalize(&Front);
// 		Camera->Front = Front;

// 		// Recreate Right and Up Vector
// 		Camera->Right 	= EM(Vector)::Normalize(&(EM(Vector)::Cross(&Camera->Front, &Camera->WorldUp)));
// 		Camera->Up 		= EM(Vector)::Normalize(&(EM(Vector)::Cross(&Camera->Right, &Camera->Front)));
// 	}

// 	//LMatrix4 LookAt( const LVector3& Eye, const LVector3& Center, const LVector3& Up )
// 	inline EM(Mat4) GetViewMatrix(struct Cam3D* Camera)
// 	{
// 		assert(Camera);

// 		EM(Vec3) Eye = Camera->Position;
// 		EM(Vec3) Center = Camera->Position + Camera->Front;
// 		EM(Vec3) Up = Camera->Up;

// 	    EM(Mat4) Matrix;
// 	    EM(Vec3) X, Y, Z;
		
// 		//Create a new coordinate system:
// 	    Z = Eye - Center;
// 	    EM(Vector)::Normalize(&Z);
// 	    Y = Up;
// 	    X = EM(Vector)::Cross(&Y, &Z);
	
// 		//Recompute Y = Z cross X:
// 	    Y = EM(Vector)::Cross(&Z, &X);
		
// 		//Cross-product gives area of parallelogram, which is < 1.0 for non-perpendicular unit-length vectors; so normalize X, Y here:

// 	    EM(Vector)::Normalize(&X);
// 	    EM(Vector)::Normalize(&Y);
		
// 		//Put everything into the resulting 4x4 matrix:

// 	    Matrix.columns[0].x = X.x;
// 	    Matrix.columns[0].y = X.y;
// 	    Matrix.columns[0].z = X.z;
// 	    Matrix.columns[0].w = -EM(Vector)::Dot(&X, &Eye);

// 	    Matrix.columns[1].x = Y.x;
// 	    Matrix.columns[1].y = Y.y;
// 	    Matrix.columns[1].z = Y.z;
// 	    Matrix.columns[1].w = -EM(Vector)::Dot(&Y, &Eye);

// 	    Matrix.columns[2].x = Z.x;
// 	    Matrix.columns[2].y = Z.y;
// 	    Matrix.columns[2].z = Z.z;
// 	    Matrix.columns[2].w = -EM(Vector)::Dot(&Z, &Eye);

// 	    Matrix.columns[3].x = 0;
// 	    Matrix.columns[3].y = 0;
// 	    Matrix.columns[3].z = 0;
// 	    Matrix.columns[3].w = 1.0f;

// 	    return Matrix;
// 	}

// 	inline void ProcessKeyboard(struct Cam3D* Camera, CameraMovement Direction)
// 	{
// 		assert(Camera);

// 		float Speed = Camera->MovementSpeed;
// 		EM(Vec3)* Position = &Camera->Position;
// 		EM(Vec3)* Front = &Camera->Front;
// 		EM(Vec3)* Right = &Camera->Right;

// 		if (Direction == FORWARD)  	*Position += *Front * Speed;
// 		if (Direction == BACKWARD) 	*Position += *Front * Speed;   	
// 		if (Direction == LEFT) 	   	*Position += *Right * Speed;   	
// 		if (Direction == RIGHT) 	*Position += *Right * Speed;   	

// 		// Update Front, Right and Up Vectors using updated Euler angles
// 		UpdateCameraVectors(Camera);
// 	}

// 	inline void ProcessMouseMovement(struct Cam3D* Camera, float XOffSet, float YOffSet, bool ConstrainPitch)
// 	{
// 		assert(Camera);

// 		XOffSet *= Camera->MouseSensitivity;
// 		YOffSet *= Camera->MouseSensitivity;

// 		Camera->Yaw   += XOffSet;
// 		Camera->Pitch += YOffSet;

// 		// Make sure that when pitch is out of bounds, screen doesn't get flipped
// 		if (ConstrainPitch)
// 		{
// 			float* Pitch = &Camera->Pitch;
// 			if (*Pitch > 89.0f)  *Pitch = 89.0f;
// 			if (*Pitch < -89.0f) *Pitch = -89.0f;
// 		}

// 		// Update Front, Right and Up Vectors using updated Euler angles
// 		UpdateCameraVectors(Camera);
// 	}

// 	inline float GetZoom(struct Cam3D* Camera)
// 	{
// 		assert(Camera);

// 		return Camera->Zoom;
// 	}


// }}}









