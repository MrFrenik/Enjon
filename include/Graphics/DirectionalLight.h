#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Base/Object.h"
#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Graphics/FrameBuffer.h"
#include "Defines.h"

namespace Enjon 
{ 
	class GraphicsScene;

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
				void SetGraphicsScene(GraphicsScene* scene);
 
				ENJON_FUNCTION()
				GraphicsScene* GetGraphicsScene( ) const;

		private:
				ENJON_PROPERTY( )
				Vec3 mDirection;

				ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f )
				ColorRGBA32 mColor;

				ENJON_PROPERTY( UIMin = 0.0f, UIMax = 100.0f )
				f32 mIntensity;

				GraphicsScene* mGraphicsScene = nullptr;

	};
}

#endif