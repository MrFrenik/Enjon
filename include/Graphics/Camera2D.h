#ifndef CAMERA2D_H
#define CAMERA2D_H

#include "Math/Maths.h"

//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////// 

namespace Enjon { namespace Graphics {

	enum class Anchor { TOPLEFT, TOP, TOPRIGHT, RIGHT, LEFT, CENTER, BOTTOMLEFT, BOTTOM, BOTTOMRIGHT};
	
	class Camera2D{

	public:
		Camera2D();
		~Camera2D();

	void Init(int screenWidth, int screenHeight);
	void Update();

	void ConvertScreenToWorld(Enjon::Math::Vec2& screenCoords);

	bool IsBoundBoxInCamView(const Enjon::Math::Vec2& position, const Enjon::Math::Vec2& dimensions);

	//Get Anchor Positions
	Math::Vec2 GetAnchorPosition(Anchor anchor);
	
	//Setters
	void SetPosition(Enjon::Math::Vec2& newPosition){m_position = newPosition; m_needsMatrixUpdate = true;}		
	void SetScale(float newScale){m_scalar = newScale; m_needsMatrixUpdate = true;}

	//Getters	
	inline Enjon::Math::Vec2 GetPosition() const {return m_position;}
	inline float GetScale() const {return m_scalar;}
	inline Enjon::Math::Mat4 GetCameraMatrix() const {return m_cameraMatrix;}

	private:
		int m_screenWidth, m_screenHeight;
		bool m_needsMatrixUpdate;
		Enjon::Math::Vec2 m_position;
		Enjon::Math::Mat4 m_cameraMatrix;
		Enjon::Math::Mat4 m_orthoMatrix;
		float m_scalar;
	};
}}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#endif