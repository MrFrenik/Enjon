#include "Graphics/SpotLight.h"


namespace Enjon { namespace Graphics {

	SpotLight::SpotLight()
	{
		Position 	= EM::Vec3(0.0f, 0.0f, 0.0f);
		Color 		= EG::RGBA16_White(); 
		Parameters 	= SpotLightParameters(0.0f, 0.0f, 0.0f, EM::Vec3(0, 0, 0), 0.0f, 0.0f);
	}

	SpotLight::SpotLight(EM::Vec3& _Position, SLParams& _Params, EG::ColorRGBA16& _Color, float _Intensity)
	{
		Position 	= _Position;
		Parameters 	= _Params;
		Color 		= _Color;
		Intensity 	= _Intensity;

		const float Constant 	= Parameters.Constant;
		const float Linear 		= Parameters.Linear;
		const float Quadratic 	= Parameters.Quadratic;
	}

	SpotLight::~SpotLight()
	{

	}

}}