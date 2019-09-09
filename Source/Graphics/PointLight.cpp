#include "Graphics/PointLight.h"
#include "Graphics/GraphicsScene.h"

namespace Enjon 
{ 
	//============================================================================================================================

	void PointLight::ExplicitConstructor()
	{
		mPosition 	= Vec3(0.0f, 0.0f, 0.0f);
		mColor 		= RGBA32_White(); 
		mAttenuationRate = 0.5f;
		mIntensity = 100.0f;
		mRadius = 100.0f;
	}

	//============================================================================================================================

	PointLight::PointLight( const Vec3& position, const f32& attenuationRate, const ColorRGBA32& color, const f32& intensity, const f32& radius )
	{
		mPosition = position;
		mAttenuationRate = attenuationRate;
		mColor = color;
		mIntensity = intensity;	
		mRadius = radius;
	}

	//============================================================================================================================

	void PointLight::ExplicitDestructor()
	{
		if (mGraphicsScene != nullptr)
		{
			mGraphicsScene->RemovePointLight(this);
		}
	}

	//============================================================================================================================

	void PointLight::SetColor( const ColorRGBA32& color )
	{
		mColor = color;
	}

	//============================================================================================================================

	void PointLight::SetIntensity( const f32& intensity )
	{
		mIntensity = intensity;	
	}

	//============================================================================================================================

	void PointLight::SetGraphicsScene( GraphicsScene* scene )
	{
		mGraphicsScene = scene;
	}

	//============================================================================================================================

	void PointLight::SetPosition( const Vec3& position )
	{
		mPosition = position;
	}

	//============================================================================================================================

	void PointLight::SetAttenuationRate( const f32& rate )
	{
		mAttenuationRate = rate;
	}

	//============================================================================================================================

	void PointLight::SetRadius( const f32& radius )
	{
		mRadius = radius;
	}

	//==============================================================================================

	GraphicsScene* PointLight::GetGraphicsScene( ) const
	{
		return mGraphicsScene;
	}

	//============================================================================================== 

	f32 PointLight::GetAttenuationRate() const 
	{ 
		return mAttenuationRate; 
	}

	//============================================================================================== 
	
	f32 PointLight::GetRadius() const
	{ 
		return mRadius; 
	}

	//============================================================================================== 

	Vec3 PointLight::GetPosition() const 	
	{ 
		return mPosition; 
	}

	//============================================================================================== 

	ColorRGBA32 PointLight::GetColor() const  
	{ 
		return mColor; 
	}

	//============================================================================================== 

	f32 PointLight::GetIntensity() const
	{ 
		return mIntensity; 
	}

	//============================================================================================== 
}