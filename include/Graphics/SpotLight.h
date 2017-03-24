#ifndef ENJON_SPOT_LIGHT_H
#define ENJON_SPOT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { 

	class Scene;

	struct SpotLightParameters
	{
		SpotLightParameters(){}
		SpotLightParameters(float constant, float linear, float quadratic, EM::Vec3 dir, float inner, float outer)
		{
			mConstant 		= constant;
			mLinear 		= linear;
			mQuadratic 		= quadratic;
			mDirection 		= dir;
			mInnerCutoff 	= inner;
			mOuterCutoff 	= outer;
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

		EM::Vec3 mDirection;
		float mInnerCutoff;
		float mOuterCutoff;
	};

	typedef struct SpotLightParameters SLParams;

	class SpotLight
	{
		public:
			SpotLight();
			SpotLight(EM::Vec3& position, SLParams& params, ColorRGBA16& color, float intensity = 1.0f);
			~SpotLight();

			EM::Vec3& GetPosition() { return mPosition; }
			ColorRGBA16& GetColor() { return mColor; }
			SLParams& GetParams() { return mParams; }
			float GetIntensity() { return mIntensity; }

			void SetPosition(EM::Vec3& position);
			void SetDirection(EM::Vec3& direction);
			void SetColor(ColorRGBA16& color);
			void SetIntensity(float intensity);
			void SetScene(Scene* scene);
			void SetParams(SLParams& params);

		private:
			EM::Vec3 			mPosition;
			SLParams 			mParams;	
			ColorRGBA16 	mColor;
			Scene* 			mScene 			= nullptr;
			float 				mIntensity;
	};

}

#endif