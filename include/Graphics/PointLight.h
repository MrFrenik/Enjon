#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { 

	class Scene;

	class PointLight
	{
		public:
			PointLight();
			PointLight(EM::Vec3& position, float attenuationRate, EG::ColorRGBA16& color, float intensity = 1.0f, float radius = 1.0f);
			~PointLight();

			EM::Vec3& GetPosition() 	{ return mPosition; }
			EG::ColorRGBA16& GetColor() { return mColor; }
			float GetIntensity() 		{ return mIntensity; }
			float GetAttenuationRate() 	{ return mAttenuationRate; }
			float GetRadius() 			{ return mRadius; }

			void SetColor(EG::ColorRGBA16& color);
			void SetIntensity(float intensity);
			void SetScene(EG::Scene* scene);
			void SetPosition(EM::Vec3& position);
			void SetAttenuationRate(float rate);
			void SetRadius(float radius);

		private:
			EM::Vec3 			mPosition;
			float 				mAttenuationRate;
			float 				mRadius;
			float 				mIntensity;
			EG::ColorRGBA16 	mColor;
			EG::Scene* 			mScene 			= nullptr;
	};

}}

#endif