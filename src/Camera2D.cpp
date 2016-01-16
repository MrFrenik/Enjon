#include <algorithm>

#include "Graphics/Camera2D.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {
	
	Camera2D::Camera2D() 
		:
		m_screenWidth(500), 
		m_screenHeight(500),
		m_needsMatrixUpdate(true), 
		m_position(0.0f, 0.0f), 
		m_cameraMatrix(1.0f),
		m_orthoMatrix(1.0f), 
		m_scalar(1.0f) 
	{}

	Camera2D::~Camera2D(){

	}

	void Camera2D::Init(int screenWidth, int screenHeight){
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
		m_orthoMatrix = Enjon::Math::Mat4::Orthographic(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight, -1, 1);
		m_shake_counter = 0.0f;
		m_shake_intensity = 0.0f;
	}

	void Camera2D::Update(){

		if(m_needsMatrixUpdate){
			
			//Create translate matrix and pass to camera
			Enjon::Math::Vec3 translate(-m_position.x + m_screenWidth/2, -m_position.y + m_screenHeight/2, 0.0f);
			m_cameraMatrix = m_orthoMatrix * Enjon::Math::Mat4::Translate(translate); 
			
			//Create scale matrix and pass to camera
			Enjon::Math::Vec3 scale(m_scalar, m_scalar, 0.0f);
			m_cameraMatrix = Enjon::Math::Mat4::Scale(scale) * m_cameraMatrix;

			//Reset matrix update bool to false
			m_needsMatrixUpdate = false;
		}

		// Check if screen is being shaken
		if (this->m_shake_counter > 0.0f)
		{
			m_shake_counter -= 0.1f;
			float XOffset = Enjon::Random::Roll(-m_shake_intensity, m_shake_intensity);
			float YOffset = Enjon::Random::Roll(-m_shake_intensity, m_shake_intensity);
			Enjon::Math::Vec2 Position = this->GetPosition();
			this->SetPosition(Enjon::Math::Vec2(Position.x + XOffset, Position.y + YOffset)); 
		}

	}

	void Camera2D::ConvertScreenToWorld(Enjon::Math::Vec2& screenCoords){

		//Invert y-direction
		screenCoords.y = m_screenHeight - screenCoords.y;

		//Convert screenCoords to where 0,0 is our center of screen	
		screenCoords -= Enjon::Math::Vec2((float)m_screenWidth / 2.0f ,(float)m_screenHeight / 2.0f);

		//Scale the coords
		screenCoords /= m_scalar;

		//Translate with camera position
		screenCoords += m_position;
	}
	
	bool Camera2D::IsBoundBoxInCamView(const Enjon::Math::Vec2& position, const Enjon::Math::Vec2& dimensions){

		Enjon::Math::Vec2 scaledScreenDimensions = Enjon::Math::Vec2((float)m_screenWidth, (float)m_screenHeight) / m_scalar;
		// const float TILE_RADIUS = (float)TILE_WIDTH / 2;
		const float MIN_DISTANCE_X = dimensions.x / 2.0f + scaledScreenDimensions.x / 2.0f;
		const float MIN_DISTANCE_Y = dimensions.y / 2.0f + scaledScreenDimensions.y / 2.0f;

		//Center position of parameters passed in
		Enjon::Math::Vec2 centerPos = position + dimensions / 2.0f;	
		//Center position of camera
		Enjon::Math::Vec2 centerCameraPos = m_position;
		//Distance vector between two center positions
		Enjon::Math::Vec2 distVec = centerPos - centerCameraPos;

		float xDepth = MIN_DISTANCE_X - abs(distVec.x);
		float yDepth = MIN_DISTANCE_Y - abs(distVec.y);

		//If true, we are colliding
		if(xDepth > 0 && yDepth > 0){
			
			//There was a collision
			return true;			
		}

		//If false, no collision
		return false;

	}	
	
	Math::Vec2 Camera2D::GetAnchorPosition(Anchor anchor)
	{
		Math::Vec2 pos(0, 0);
		switch(anchor)
		{
			case Anchor::TOPLEFT:
				pos.x = m_position.x - m_screenWidth / 2.0f;
				pos.y = m_position.y + m_screenHeight / 2.0f;
				break;
			default:
				break;
		}

		return pos;
	}

	void Camera2D::ShakeScreen(float Intensity)
	{
		this->m_shake_intensity = Intensity;
		this->m_shake_counter = 1.0f;
	}

}}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





















