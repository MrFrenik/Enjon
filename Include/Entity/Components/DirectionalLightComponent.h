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
			virtual void PostConstruction( ) override;

			/*
			* @brief
			*/
			virtual void Update() override;

			/*
			* @brief
			*/
			virtual void UpdateTransform( const Transform& transform ) override;

			/**
			* @brief
			*/
			virtual Result OnEditorUI( ) override;

			/*
			* @brief
			*/
			DirectionalLight& GetLight() { return *mLight; } 

			/*
			* @brief
			*/
			ColorRGBA32& GetColor() { return mLight->GetColor(); }

			/*
			* @brief
			*/
			f32 GetIntensity() { return mLight->GetIntensity(); } 

			/*
			* @brief
			*/
			void SetAttenuationRate( const f32& rate );

			/*
			* @brief
			*/
			void SetPosition( const Vec3& position );

			/*
			* @brief
			*/
			void SetColor( const ColorRGBA32& color );

			/* 
			* @brief
			*/
			void SetIntensity( const f32& intensity );

			/* 
			* @brief
			*/
			void SetRadius( const f32& radius ); 

		private:
			//ENJON_PROPERTY( )
			//DirectionalLight mLight;	

			ResourceHandle< DirectionalLight > mLight;
	};
}

#endif
