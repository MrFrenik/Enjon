#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { namespace Graphics {

	class DirectionalLight
	{
		public:
				DirectionalLight();
				DirectionalLight(EM::Vec3& Direction, EG::ColorRGBA16& Color, float Intensity = 1.0f);
				~DirectionalLight();
				void BindDepth();
				void UnbindDepth();

				EM::Vec3 			Direction;
				EG::ColorRGBA16 	Color;
				float 				Intensity;
				GLuint DepthMap;
				static const int DirectionalLightShadowResolution;

			private:
				void InitDepthMap();
	};
}}

#endif