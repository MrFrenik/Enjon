#include "Graphics/PointLight.h"


namespace Enjon { namespace Graphics {

	PointLight::PointLight()
	{
		Position 	= EM::Vec3(0.0f, 0.0f, 0.0f);
		Color 		= EG::RGBA16_White(); 
		Parameters 	= PointLightParameters(0.0f, 0.0f, 0.0f);
	}

	PointLight::PointLight(EM::Vec3& _Position, PLParams& _Params, EG::ColorRGBA16& _Color, float _Intensity)
	{
		Position 	= _Position;
		Parameters 	= _Params;
		Color 		= _Color;
		Intensity 	= _Intensity;

		const float Constant 	= Parameters.Constant;
		const float Linear 		= Parameters.Linear;
		const float Quadratic 	= Parameters.Quadratic;

		// Calculate radius
	    float MaxBrightness = std::fmaxf(std::fmaxf(Color.r, Color.g), Color.b);
	    Parameters.Radius = (-Linear + std::sqrtf(Linear * Linear - 4 * Quadratic * (Constant - (256.0f / 5.0f) * MaxBrightness))) / (2 * Quadratic);
	}

	PointLight::~PointLight()
	{

	}

}}