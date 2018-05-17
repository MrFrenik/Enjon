#ifndef ENJON_SPOT_LIGHT_H
#define ENJON_SPOT_LIGHT_H
#pragma once

#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

#include "Base/Object.h"

namespace Enjon 
{ 
	class GraphicsScene;

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
		ENJON_CLASS_BODY( SpotLight )

		public:

			/** 
			* @brief
			*/
			virtual void ExplicitConstructor( ) override;

			/** 
			* @brief
			*/
			SpotLight(Vec3& position, SLParams& params, ColorRGBA32& color, float intensity = 1.0f); 

			/** 
			* @brief
			*/
			Vec3& GetPosition() { return mPosition; }

			/** 
			* @brief
			*/
			ColorRGBA32& GetColor() { return mColor; }

			/** 
			* @brief
			*/
			SLParams& GetParams() { return mParams; }

			/** 
			* @brief
			*/
			float GetIntensity() { return mIntensity; }

			/** 
			* @brief
			*/
			void SetPosition(Vec3& position);

			/** 
			* @brief
			*/
			void SetDirection(Vec3& direction);

			/** 
			* @brief
			*/
			void SetColor(ColorRGBA32& color);

			/** 
			* @brief
			*/
			void SetIntensity(float intensity);

			/** 
			* @brief
			*/
			void SetGraphicsScene(GraphicsScene* scene);

			/** 
			* @brief
			*/
			void SetParams(SLParams& params);

		private:
			ENJON_PROPERTY( Editable )
			Vec3 mPosition;

			SLParams mParams;	
			
			ENJON_PROPERTY( Editable )
			f32 mIntensity;

			ENJON_PROPERTY( Editable )
			ColorRGBA32 mColor;

			GraphicsScene* mGraphicsScene = nullptr;
	};

}

#endif