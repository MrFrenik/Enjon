#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTarget.h"
#include "Defines.h"

namespace Enjon { namespace Graphics {

	class Scene;

	class DirectionalLight
	{
		public:
				DirectionalLight();
				DirectionalLight(EM::Vec3& direction, 
								EG::ColorRGBA16& color, 
								float intensity = 1.0f);
				~DirectionalLight();

				EM::Vec3* GetDirection() { return &mDirection; }
				EG::ColorRGBA16* GetColor() { return &mColor; }
				float GetIntensity() { return mIntensity; }

				void SetDirection(EM::Vec3& direction);
				void SetColor(EG::ColorRGBA16& color);
				void SetIntensity(float intensity);
				void SetScene(EG::Scene* scene);

		private:
				EM::Vec3 			mDirection;
				EG::ColorRGBA16 	mColor;
				EG::Scene* 			mScene 			= nullptr;
				float 				mIntensity;
	};
}}

#endif