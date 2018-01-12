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

	ENJON_CLASS( Construct )
	class DirectionalLight : public Enjon::Object
	{
		ENJON_CLASS_BODY( )

		public:
				DirectionalLight();
				DirectionalLight(Vec3& direction, 
								ColorRGBA32& color, 
								float intensity = 1.0f);
				~DirectionalLight();

				ENJON_FUNCTION()
				Vec3 GetDirection() const { return mDirection; }

				ENJON_FUNCTION()
				ColorRGBA32 GetColor() const { return mColor; }

				ENJON_FUNCTION()
				f32 GetIntensity() const { return mIntensity; }

				ENJON_FUNCTION()
				void SetDirection(const Vec3& direction);

				ENJON_FUNCTION()
				void SetColor(const ColorRGBA32& color);

				ENJON_FUNCTION()
				void SetIntensity(float intensity);

				ENJON_FUNCTION()
				void SetScene(Scene* scene);

		private:
				ENJON_PROPERTY( )
				Vec3 mDirection;

				ENJON_PROPERTY( )
				ColorRGBA32 mColor;

				ENJON_PROPERTY( )
				f32 mIntensity;

				Scene* mScene = nullptr;

	};
}

#endif