#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { 

	struct PointLightParameters
	{
		PointLightParameters(){}
		PointLightParameters(float _Constant, float _Linear, float _Quadratic)
		{
			Constant 	= _Constant;
			Linear 		= _Linear;
			Quadratic 	= _Quadratic;
			Radius 		= 1.0f;
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

		float Radius;	
	};

	typedef struct PointLightParameters PLParams;

	struct PointLight
	{
			PointLight();
			PointLight(EM::Vec3& Position, PLParams& Params, EG::ColorRGBA16& Color, float Intensity = 1.0f);
			~PointLight();

			EM::Vec3 			Position;
			PLParams 			Parameters;	
			EG::ColorRGBA16 	Color;
			float 				Intensity;
	};

}}

#endif