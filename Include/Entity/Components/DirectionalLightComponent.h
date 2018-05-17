#pragma once
#ifndef ENJON_DIRECTIONAL_LIGHT_COMPONENT_H
#define ENJON_DIRECTIONAL_LIGHT_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/DirectionalLight.h"

namespace Enjon
{
	ENJON_CLASS( )
	class DirectionalLightComponent : public Component
	{ 
		ENJON_COMPONENT( DirectionalLightComponent )

		public:

			/*
			* @brief
			*/
			virtual void ExplicitConstructor() override;

			/*
			* @brief
			*/
			virtual void ExplicitDestructor() override;

			/*
			* @brief
			*/
			virtual void Update() override;

			/*
			* @brief
			*/
			DirectionalLight* GetLight() { return &mLight; } 

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

		private:
			ENJON_PROPERTY( )
			DirectionalLight mLight;	
	};
}

#endif
