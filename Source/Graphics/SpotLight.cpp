#include "Graphics/SpotLight.h"
#include "Graphics/GraphicsScene.h"


namespace Enjon 
{
	//============================================================================================================================

	void SpotLight::ExplicitConstructor()
	{
		mPosition 	= Vec3( 0.0f, 0.0f, 0.0f );
		mColor 		= RGBA32_White(); 
		mParams 	= SpotLightParameters( 0.0f, 0.0f, 0.0f, Vec3(0, 0, 0), 0.0f, 0.0f );
	}

	//============================================================================================================================

	SpotLight::SpotLight( const Vec3& position, const SLParams& params, const ColorRGBA32& color, const f32& intensity )
	{
		mPosition 	= position;
		mParams 	= params;
		mColor 		= color;
		mIntensity 	= intensity;

		const f32 constant 		= mParams.mConstant;
		const f32 linear 		= mParams.mLinear;
		const f32 quadratic 	= mParams.mQuadratic;

		// Not really doing anything with these for some reason...
	} 
	
	//============================================================================================================================

	void SpotLight::SetColor( const ColorRGBA32& color )
	{
		mColor = color;
	}
	
	//============================================================================================================================

	void SpotLight::SetIntensity( const f32& intensity )
	{
		mIntensity = intensity;
	}
	
	//============================================================================================================================

	void SpotLight::SetGraphicsScene( GraphicsScene* scene )
	{
		mGraphicsScene = scene;	
	}
	
	//============================================================================================================================

	void SpotLight::SetPosition( const Vec3& position )
	{
		mPosition = position;
	}
	
	//============================================================================================================================

	void SpotLight::SetDirection( const Vec3& direction )
	{
		mParams.mDirection = direction;
	}
	
	//============================================================================================================================

	void SpotLight::SetParams( const SLParams& params )
	{
		mParams = params;
	}
	
	//============================================================================================================================
}