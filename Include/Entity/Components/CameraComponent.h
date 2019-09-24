#pragma once
#ifndef ENJON_CAMERA_COMPONENT_H
#define ENJON_CAMERA_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsScene.h"
#include "System/Types.h"

namespace Enjon
{
	ENJON_CLASS( Construct )
	class CameraComponent : public Component
	{
		ENJON_COMPONENT( CameraComponent )

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
			virtual Result OnViewportDebugDraw() override;

			/*
			* @brief
			*/
			virtual void Update( ) override; 

			/*
			* @brief
			*/
			virtual void Initialize( ) override;

			/*
			* @brief
			*/
			Camera* GetCamera( );
			
			/* 
			* @brief Get scene of renderable 
			*/
			GraphicsScene* GetGraphicsScene() const; 
 
			/* 
			* @brief Set scene of renderable 
			*/
			void SetGraphicsScene(GraphicsScene* scene); 

			/* 
			* @brief Set scene of renderable 
			*/
			void SetActiveCamera( );

			/* 
			* @brief Override for editor ui
			*/
			Result OnEditorUI( ) override;

		private: 
			
			ENJON_PROPERTY( )
			Camera mCamera;
	};
}

#endif
