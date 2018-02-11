#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Base/Object.h"
#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { 

	class GraphicsScene;

	ENJON_CLASS( Construct )
	class PointLight : public Enjon::Object
	{
		ENJON_CLASS_BODY ( ) 

		public:
			ENJON_FUNCTION( Constructor ) 
			PointLight();
			
			ENJON_FUNCTION( Constructor ) 
			PointLight(Vec3& position, float attenuationRate, ColorRGBA32& color, float intensity = 1.0f, float radius = 1.0f);

			ENJON_FUNCTION( Destructor ) 
			~PointLight();

			ENJON_FUNCTION() 
			Vec3 GetPosition() const 	
			{ 
				return mPosition; 
			}

			ENJON_FUNCTION() 
			ColorRGBA32 GetColor() const  
			{ 
				return mColor; 
			}

			ENJON_FUNCTION() 
			float GetIntensity() const
			{ 
				return mIntensity; 
			}

			ENJON_FUNCTION() 
			float GetAttenuationRate() const 
			{ 
				return mAttenuationRate; 
			}
			
			ENJON_FUNCTION() 
			float GetRadius() const
			{ 
				return mRadius; 
			}

			ENJON_FUNCTION() 
			void SetColor(ColorRGBA32& color);

			ENJON_FUNCTION() 
			void SetIntensity(float intensity);
			
			ENJON_FUNCTION() 
			void SetGraphicsScene(GraphicsScene* scene);
			
			ENJON_FUNCTION() 
			void SetPosition(Enjon::Vec3& position);
			
			ENJON_FUNCTION() 
			void SetAttenuationRate(float rate);

			ENJON_FUNCTION() 
			void SetRadius(float radius); 

			/*
			* @brief
			*/
			GraphicsScene* GetGraphicsScene( ) const; 

		private: 

			ENJON_PROPERTY( Editable, UIMin = -100.0f, UIMax = 100.0f )
			Vec3 mPosition;

			ENJON_PROPERTY( Editable )
			f32 mAttenuationRate;

			ENJON_PROPERTY( Editable, UIMin = 1.0f, UIMax = 250.0f )
			f32 mRadius;

			ENJON_PROPERTY( Editable, UIMin = 0.0f, UIMax = 100.0f )
			f32 mIntensity;

			ENJON_PROPERTY( Editable, UIMin = 0.0f, UIMax = 1.0f )
			ColorRGBA32 mColor;
			
			GraphicsScene* mGraphicsScene = nullptr;
	};

}

#endif