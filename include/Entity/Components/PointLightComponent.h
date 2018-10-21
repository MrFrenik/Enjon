#pragma once
#ifndef ENJON_POINT_LIGHT_COMPONENT_H
#define ENJON_POINT_LIGHT_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/PointLight.h"

namespace Enjon
{
	ENJON_CLASS( )
	class PointLightComponent : public Component
	{ 
		ENJON_COMPONENT( PointLightComponent )

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
			virtual void PostConstruction( ) override;

			/*
			* @brief
			*/
			virtual void Update() override;

			/**
			* @brief
			*/
			virtual Result OnEditorUI( ) override;

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

		private:
			ENJON_PROPERTY( )
			PointLight mLight;	
	};
}

#endif