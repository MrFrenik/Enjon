#ifndef ENJON_DIRECTIONAL_LIGHT_H
#define ENJON_DIRECTIONAL_LIGHT_H
#pragma once

#include "Base/Object.h"
#include "Math/Vec3.h"
#include "Graphics/Color.h"
#include "Graphics/FrameBuffer.h"
#include "Defines.h"

namespace Enjon 
{ 
	class GraphicsScene;

	ENJON_CLASS( )
	class DirectionalLight : public Enjon::Object
	{
		ENJON_CLASS_BODY( DirectionalLight )

		public:

			/**
			* @brief
			*/
			virtual void ExplicitConstructor( ) override; 

			/**
			* @brief
			*/
			DirectionalLight( const Vec3& direction, const ColorRGBA32& color, const f32& intensity = 1.0f );

			/**
			* @brief
			*/
			ENJON_FUNCTION()
			Vec3 GetDirection() const { return mDirection; }

			/**
			* @brief
			*/
			ENJON_FUNCTION()
			ColorRGBA32 GetColor() const { return mColor; }

			/**
			* @brief
			*/
			ENJON_FUNCTION()
			f32 GetIntensity() const { return mIntensity; }

			/**
			* @brief
			*/
			ENJON_FUNCTION()
			void SetDirection( const Vec3& direction );

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
			GraphicsScene* GetGraphicsScene( ) const;

		private:
				ENJON_PROPERTY( )
				Vec3 mDirection;

				ENJON_PROPERTY( UIMin = 0.0f, UIMax = 1.0f )
				ColorRGBA32 mColor;

				ENJON_PROPERTY( UIMin = 0.0f, UIMax = 100.0f )
				f32 mIntensity;

				GraphicsScene* mGraphicsScene = nullptr;

	};
}

#endif