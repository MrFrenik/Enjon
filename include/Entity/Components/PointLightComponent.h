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

			/*
			* @brief
			*/
			virtual void UpdateTransform( const Transform& transform );

			/**
			* @brief
			*/
			virtual Result OnEditorUI( ) override;

			/*
			* @brief
			*/
			PointLight* GetLight() 
			{ 
				return mLight.get_raw_ptr(); 
			}

			/*
			* @brief
			*/
			Vec3 GetPosition() const 
			{ 
				return mLight->GetPosition(); 
			}

			/*
			* @brief
			*/
			ColorRGBA32 GetColor() 
			{ 
				return mLight->GetColor(); 
			}

			/*
			* @brief
			*/
			f32 GetIntensity() 
			{ 
				return mLight->GetIntensity(); 
			}

			/*
			* @brief
			*/
			f32 GetRadius() 
			{ 
				return mLight->GetRadius(); 
			}

			/*
			* @brief
			*/
			f32 GetAttenuationRate() 
			{ 
				return mLight->GetAttenuationRate(); 
			}

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
			//PointLight mLight;	

			ResourceHandle< PointLight > mLight;
	};
}

#endif