#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { 

	class Scene;

	struct PointLightParameters
	{
		PointLightParameters()
			: mConstant(1.0f), mLinear(0.1f), mQuadratic(0.01f)
		{}
		PointLightParameters(float constant, float linear, float quadratic)
		{
			mConstant 	= constant;
			mLinear 	= linear;
			mQuadratic 	= quadratic;
			mRadius 	= 1.0f;
		}

		union
		{
			EM::Vec3 mFalloff;
			struct
			{
				float mConstant; 
				float mLinear; 
				float mQuadratic;
			};
		};

		float mRadius;	
	};

	typedef struct PointLightParameters PLParams;

	class PointLight
	{
		public:
			PointLight();
			PointLight(EM::Vec3& Position, PLParams& Params, EG::ColorRGBA16& Color, float Intensity = 1.0f);
			~PointLight();

			EM::Vec3& GetPosition() { return mPosition; }
			EG::ColorRGBA16& GetColor() { return mColor; }
			PLParams& GetParams() { return mParams; }
			float GetIntensity() { return mIntensity; }

			void SetColor(EG::ColorRGBA16& color);
			void SetIntensity(float intensity);
			void SetScene(EG::Scene* scene);
			void SetPosition(EM::Vec3& position);
			void SetParams(PLParams& params);

		private:
			EM::Vec3 			mPosition;
			PLParams 			mParams;	
			EG::ColorRGBA16 	mColor;
			EG::Scene* 			mScene 			= nullptr;
			float 				mIntensity;
	};

}}

#endif