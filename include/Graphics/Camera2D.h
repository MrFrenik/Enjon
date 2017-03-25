#ifndef ENJON_CAMERA2D_H
#define ENJON_CAMERA2D_H

#include "Math/Maths.h"
#include "Math/Random.h"

//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////// 

namespace Enjon {

	enum class Anchor { TOPLEFT, TOP, TOPRIGHT, RIGHT, LEFT, CENTER, BOTTOMLEFT, BOTTOM, BOTTOMRIGHT};
	
	class Camera2D{

	public:
		Camera2D();
		~Camera2D();

	void Init(int screenWidth, int screenHeight);
	void Update();

	void ConvertScreenToWorld(Enjon::Vec2& screenCoords);

	bool IsBoundBoxInCamView(const Enjon::Vec2& position, const Enjon::Vec2& dimensions);

	//Get Anchor Positions
	Enjon::Vec2 GetAnchorPosition(Anchor anchor);
	
	//Setters
	void SetPosition(Enjon::Vec2& newPosition){m_position = newPosition; m_needsMatrixUpdate = true;}		
	void SetScale(float newScale){m_scalar = newScale; m_needsMatrixUpdate = true;}

	//Getters	
	inline Enjon::Vec2 GetPosition() const {return m_position;}
	inline float GetScale() const {return m_scalar;}
	inline Enjon::Mat4 GetCameraMatrix() const {return m_cameraMatrix;}

	// Screen Shake
	void ShakeScreen(float Intensity);

	private:
		int m_screenWidth, m_screenHeight;
		bool m_needsMatrixUpdate;
		Vec2 m_position;
		Mat4 m_cameraMatrix;
		Mat4 m_orthoMatrix;
		f32 m_scalar;
		f32 m_shake_counter;
		f32 m_shake_intensity;
	};
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#endif