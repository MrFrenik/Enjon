#ifndef ENJON_SPOT_LIGHT_H
#define ENJON_SPOT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

#include "Base/Object.h"

namespace Enjon { 

	class Scene;

	struct SpotLightParameters
	{
		SpotLightParameters(){}
		SpotLightParameters(float constant, float linear, float quadratic, Vec3 dir, float inner, float outer)
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
			Vec3 mFalloff;
			struct
			{
				float mConstant; 
				float mLinear; 
				float mQuadratic;
			};
		};

		Vec3 mDirection;
		float mInnerCutoff;
		float mOuterCutoff;
	};

	typedef struct SpotLightParameters SLParams;

	ENJON_CLASS( )
	class SpotLight : public Enjon::Object
	{
		ENJON_CLASS_BODY( )

		public:
			SpotLight();
			SpotLight(Vec3& position, SLParams& params, ColorRGBA32& color, float intensity = 1.0f);
			~SpotLight();

			Vec3& GetPosition() { return mPosition; }
			ColorRGBA32& GetColor() { return mColor; }
			SLParams& GetParams() { return mParams; }
			float GetIntensity() { return mIntensity; }

			void SetPosition(Vec3& position);
			void SetDirection(Vec3& direction);
			void SetColor(ColorRGBA32& color);
			void SetIntensity(float intensity);
			void SetScene(Scene* scene);
			void SetParams(SLParams& params);

		private:
			ENJON_PROPERTY( Editable )
			Vec3 mPosition;

			SLParams mParams;	
			
			ENJON_PROPERTY( Editable )
			f32 mIntensity;

			ENJON_PROPERTY( Editable )
			ColorRGBA32 mColor;

			Scene* mScene = nullptr;
	};

}

#endif