#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTarget.h"
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
				void InitDepthMap();

				inline GLint GetDepthBuffer() { return DepthTarget.GetTexture(); }

				EM::Vec3 			Position;
				EG::ColorRGBA16 	Color;
				float 				Intensity;
				GLuint 				DepthFBO;
				GLuint				DepthBuffer;

				RenderTarget 		DepthTarget;

				static const int DirectionalLightShadowResolution;
				static const float DirectionalLightNear;
				static const float DirectionalLightFar;
	};
}}

#endif