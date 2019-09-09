#include "Graphics/DirectionalLight.h"
#include "Graphics/GraphicsScene.h"
#include "Math/Mat4.h"

namespace Enjon 
{ 
	//==============================================================================

	void DirectionalLight::ExplicitConstructor()
	{
		mDirection = Vec3(1, 1, 1);
		mColor = RGBA32_White();
		mIntensity = 1.0f;
	}

	//==============================================================================

	DirectionalLight::DirectionalLight( const Vec3& direction, const ColorRGBA32& color, const f32& intensity )
		: mDirection(direction), mColor(color), mIntensity(intensity)
	{
	} 

	//==============================================================================

	void DirectionalLight::SetDirection( const Vec3& direction )
	{
		mDirection = direction;
	}

	//==============================================================================

	void DirectionalLight::SetColor( const ColorRGBA32& color )
	{
		mColor = color;
	}

	//==============================================================================

	void DirectionalLight::SetIntensity( const f32& intensity )
	{
		mIntensity = intensity;
	}

	//==============================================================================

	void DirectionalLight::SetGraphicsScene( GraphicsScene* scene )
	{
		mGraphicsScene = scene;
	}

	//==============================================================================

	GraphicsScene* DirectionalLight::GetGraphicsScene( ) const
	{
		return mGraphicsScene;
	}

	//============================================================================== 
}


















