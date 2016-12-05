#ifndef ENJON_SPOT_LIGHT_H
#define ENJON_SPOT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { 

	struct SpotLightParameters
	{
		SpotLightParameters(){}
		SpotLightParameters(float _Constant, float _Linear, float _Quadratic, EM::Vec3 _Dir, float _Inner, float _Outer)
		{
			Constant 	= _Constant;
			Linear 		= _Linear;
			Quadratic 	= _Quadratic;
			Direction 	= _Dir;
			InnerCutoff = _Inner;
			OuterCutoff = _Outer;
		}

		union
		{
			EM::Vec3 Falloff;
			struct
			{
				float Constant; 
				float Linear; 
				float Quadratic;
			};
		};

		EM::Vec3 Direction;
		float InnerCutoff;
		float OuterCutoff;
	};

	typedef struct SpotLightParameters SLParams;

	struct SpotLight
	{
			SpotLight();
			SpotLight(EM::Vec3& Position, SLParams& Params, EG::ColorRGBA16& Color, float Intensity = 1.0f);
			~SpotLight();

			EM::Vec3 			Position;
			SLParams 			Parameters;	
			EG::ColorRGBA16 	Color;
			float 				Intensity;
	};

}}

#endif