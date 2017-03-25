#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTarget.h"
#include "Defines.h"

namespace Enjon {

	class Scene;

	class DirectionalLight
	{
		public:
				DirectionalLight();
				DirectionalLight(Vec3& direction, 
								ColorRGBA16& color, 
								float intensity = 1.0f);
				~DirectionalLight();

				Vec3 GetDirection() { return mDirection; }
				ColorRGBA16 GetColor() { return mColor; }
				float GetIntensity() { return mIntensity; }

				void SetDirection(Vec3& direction);
				void SetColor(ColorRGBA16& color);
				void SetIntensity(float intensity);
				void SetScene(Scene* scene);

		private:
				Vec3 			mDirection;
				ColorRGBA16 	mColor;
				Scene* 			mScene 			= nullptr;
				float 				mIntensity;
	};
}

#endif