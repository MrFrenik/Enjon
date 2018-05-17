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

	DirectionalLight::DirectionalLight(Vec3& direction, ColorRGBA32& color, float intensity)
		: mDirection(direction), mColor(color), mIntensity(intensity)
	{
	} 

	//==============================================================================

	void DirectionalLight::SetDirection(const Vec3& direction)
	{
		mDirection = direction;
	}

	//==============================================================================

	void DirectionalLight::SetColor(const ColorRGBA32& color)
	{
		mColor = color;
	}

	//==============================================================================

	void DirectionalLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;
	}

	//==============================================================================

	void DirectionalLight::SetGraphicsScene(GraphicsScene* scene)
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


















