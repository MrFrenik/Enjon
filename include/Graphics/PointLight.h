#ifndef ENJON_POINT_LIGHT_H
#define ENJON_POINT_LIGHT_H
#pragma once

#include "Base/Object.h"
#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Defines.h"

namespace Enjon { 

	class GraphicsScene;

	ENJON_CLASS( )
	class PointLight : public Enjon::Object
	{
		ENJON_CLASS_BODY ( PointLight ) 

		public:

			/**
			* @brief
			*/
			virtual void ExplicitConstructor( );

			/**
			* @brief
			*/
			virtual void ExplicitDestructor( );
			
			/**
			* @brief
			*/
			ENJON_FUNCTION( Constructor ) 
			PointLight( const Vec3& position, const f32& attenuationRate, const ColorRGBA32& color, const f32& intensity = 1.0f, const f32& radius = 1.0f); 

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			Vec3 GetPosition( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			ColorRGBA32 GetColor( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			float GetIntensity( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			float GetAttenuationRate( ) const;
			
			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			float GetRadius( ) const;

			/**
			* @brief
			*/
			ENJON_FUNCTION() 
			void SetColor( const ColorRGBA32& color );

			/**
			* @brief
			*/
			ENJON_FUNCTION() 
			void SetIntensity( const f32& intensity );
			
			/**
			* @brief
			*/
			ENJON_FUNCTION() 
			void SetGraphicsScene( GraphicsScene* scene );
			
			/**
			* @brief
			*/
			ENJON_FUNCTION() 
			void SetPosition( const Vec3& position );
			
			/**
			* @brief
			*/
			ENJON_FUNCTION() 
			void SetAttenuationRate( const f32& rate );

			/**
			* @brief
			*/
			ENJON_FUNCTION() 
			void SetRadius( const f32& radius ); 

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