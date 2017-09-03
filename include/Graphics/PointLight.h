#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Base/Object.h"
#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { 

	class Scene;

	ENJON_CLASS( )
	class PointLight : public Enjon::Object
	{
		ENJON_CLASS_BODY ( PointLight ) 

		public:
			ENJON_FUNCTION( Constructor ) 
			PointLight();
			
			ENJON_FUNCTION( Constructor ) 
			PointLight(Vec3& position, float attenuationRate, ColorRGBA16& color, float intensity = 1.0f, float radius = 1.0f);

			ENJON_FUNCTION( Destructor ) 
			~PointLight();

			ENJON_FUNCTION() 
			Vec3 GetPosition() 	{ return mPosition; }

			ENJON_FUNCTION() 
			ColorRGBA16 GetColor() { return mColor; }

			ENJON_FUNCTION() 
			float GetIntensity() 		{ return mIntensity; }

			ENJON_FUNCTION() 
			float GetAttenuationRate() 	{ return mAttenuationRate; }
			
			ENJON_FUNCTION() 
			float GetRadius() 			{ return mRadius; }

			ENJON_FUNCTION() 
			void SetColor(ColorRGBA16& color);

			ENJON_FUNCTION() 
			void SetIntensity(float intensity);
			
			ENJON_FUNCTION() 
			void SetScene(Scene* scene);
			
			ENJON_FUNCTION() 
			void SetPosition(Enjon::Vec3& position);
			
			ENJON_FUNCTION() 
			void SetAttenuationRate(float rate);

			ENJON_FUNCTION() 
			void SetRadius(float radius); 

		private: 

			ENJON_PROPERTY( Editable )
			Vec3 mPosition;

			ENJON_PROPERTY( Editable )
			f32 mAttenuationRate;

			ENJON_PROPERTY( Editable )
			f32 mRadius;

			ENJON_PROPERTY( Editable )
			f32 mIntensity;

			ENJON_PROPERTY( Editable )
			ColorRGBA16 mColor;
			
			Scene* mScene = nullptr;
	};

}

#endif