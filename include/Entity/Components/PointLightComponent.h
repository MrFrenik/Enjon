#pragma once
#ifndef ENJON_POINT_LIGHT_COMPONENT_H
#define ENJON_POINT_LIGHT_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/PointLight.h"

namespace Enjon
{
	ENJON_CLASS( Construct )
	class PointLightComponent : public Component
	{
		ENJON_CLASS_BODY( ) 

		ENJON_COMPONENT( PointLightComponent )

		public:

			/*
			* @brief
			*/
			PointLightComponent();

			/*
			* @brief
			*/
			~PointLightComponent();

			/*
			* @brief
			*/
			virtual void Update( const f32& dt) override;

			/*
			* @brief
			*/
			PointLight* GetLight() { return &mLight; }

			/*
			* @brief
			*/
			Vec3& GetPosition() const 
			{ 
				return mLight.GetPosition(); 
			}

			/*
			* @brief
			*/
			ColorRGBA32& GetColor() { return mLight.GetColor(); }

			/*
			* @brief
			*/
			float GetIntensity() { return mLight.GetIntensity(); }

			/*
			* @brief
			*/
			float GetRadius() { return mLight.GetRadius(); }

			/*
			* @brief
			*/
			float GetAttenuationRate() { return mLight.GetAttenuationRate(); }

			/*
			* @brief
			*/
			void SetAttenuationRate(float rate);

			/*
			* @brief
			*/
			void SetPosition(Vec3& position);

			/*
			* @brief
			*/
			void SetColor(ColorRGBA32& color);

			/* 
			* @brief
			*/
			void SetIntensity(float intensity);

			/* 
			* @brief
			*/
			void SetRadius(float radius); 

			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override;

		private:
			PointLight mLight;	
	};
}

#endif