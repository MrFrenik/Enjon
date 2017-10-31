#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Base/Object.h"
#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Graphics/RenderTarget.h"
#include "Defines.h"

namespace Enjon {

	class Scene;

	ENJON_CLASS( )
	class DirectionalLight : public Enjon::Object
	{
		ENJON_CLASS_BODY( )

		public:
				DirectionalLight();
				DirectionalLight(Vec3& direction, 
								ColorRGBA16& color, 
								float intensity = 1.0f);
				~DirectionalLight();

				ENJON_FUNCTION()
				Vec3 GetDirection() { return mDirection; }

				ENJON_FUNCTION()
				ColorRGBA16 GetColor() { return mColor; }

				ENJON_FUNCTION()
				f32 GetIntensity() { return mIntensity; }

				ENJON_FUNCTION()
				void SetDirection(Vec3& direction);

				ENJON_FUNCTION()
				void SetColor(ColorRGBA16& color);

				ENJON_FUNCTION()
				void SetIntensity(float intensity);

				ENJON_FUNCTION()
				void SetScene(Scene* scene);

		private:
				ENJON_PROPERTY( )
				Vec3 mDirection;

				ENJON_PROPERTY( )
				ColorRGBA16 mColor;

				ENJON_PROPERTY( )
				f32 mIntensity;

				Scene* mScene = nullptr;

	};
}

#endif