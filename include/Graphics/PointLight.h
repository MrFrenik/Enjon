#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { 

	class Scene;

	class PointLight
	{
		public:
			PointLight();
			PointLight(EM::Vec3& position, float attenuationRate, ColorRGBA16& color, float intensity = 1.0f, float radius = 1.0f);
			~PointLight();

			EM::Vec3& GetPosition() 	{ return mPosition; }
			ColorRGBA16& GetColor() { return mColor; }
			float GetIntensity() 		{ return mIntensity; }
			float GetAttenuationRate() 	{ return mAttenuationRate; }
			float GetRadius() 			{ return mRadius; }

			void SetColor(ColorRGBA16& color);
			void SetIntensity(float intensity);
			void SetScene(Scene* scene);
			void SetPosition(EM::Vec3& position);
			void SetAttenuationRate(float rate);
			void SetRadius(float radius);

		private:
			EM::Vec3 			mPosition;
			float 				mAttenuationRate;
			float 				mRadius;
			float 				mIntensity;
			ColorRGBA16 	mColor;
			Scene* 			mScene 			= nullptr;
	};

}

#endif