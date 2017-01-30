#include "Graphics/PointLight.h"
#include "Graphics/Scene.h"

namespace Enjon { namespace Graphics {

	PointLight::PointLight()
	{
		mPosition 	= EM::Vec3(0.0f, 0.0f, 0.0f);
		mColor 		= EG::RGBA16_White(); 
		mParams 	= PointLightParameters(0.0f, 0.0f, 0.0f);
	}

	PointLight::PointLight(EM::Vec3& position, PLParams& params, EG::ColorRGBA16& color, float intensity)
	{
		mPosition 	= position;
		mParams 	= params;
		mColor 		= color;
		mIntensity 	= intensity;

		const float constant 	= params.mConstant;
		const float linear 		= params.mLinear;
		const float quadratic 	= params.mQuadratic;

		// Calculate radius
	    float maxBrightness = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
	    params.mRadius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2 * quadratic);
	}

	void PointLight::SetColor(EG::ColorRGBA16& color)
	{
		mColor = color;
	}

	void PointLight::SetIntensity(float intensity)
	{
		mIntensity = intensity;	
	}

	void PointLight::SetScene(EG::Scene* scene)
	{
		mScene = scene;
	}

	void PointLight::SetPosition(EM::Vec3& position)
	{
		mPosition = position;
	}

	PointLight::~PointLight()
	{
	}

}}