// File: EditorSceneView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_SCENE_VIEW_H
#define ENJON_EDITOR_SCENE_VIEW_H 

#include "EditorView.h"

#include <Math/Vec2.h>
#include <Math/Transform.h>

namespace Enjon
{
	class EditorSceneView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorSceneView( EditorApp* app, Window* window );

			/**
			* @brief
			*/
			~EditorSceneView( ) = default; 

			/**
			* @brief
			*/
			Vec2 GetSceneViewProjectedCursorPosition( );

			/**
			* @brief
			*/
			Vec2 GetCenterOfViewport( );

			void UpdateCamera( );
 
		protected: 

			/**
			* @brief Must be overriden
			*/
			virtual void UpdateView( ) override;

			/**
			* @brief Must be overriden
			*/
			virtual void ProcessViewInput( ) override;

			/**
			* @brief Must be overriden
			*/
			virtual void Initialize( ) override; 

			/**
			* @brief
			*/
			virtual void CaptureState( );

			/**
			* @brief
			*/
			void HandleAssetDrop( );

			/**
			* @brief
			*/
			void RenderToolBar( );

		protected: 
			Vec2 mSceneViewWindowPosition;
			Vec2 mSceneViewWindowSize; 
			f32 mCameraSpeed = 10.f;
			f32 mMouseSensitivity = 10.0f;
			Transform mPreviousCameraTransform;
			Vec2 mMouseCoordsDelta = Vec2( 0.0f );
			bool mStartedFocusing = false;
			bool mFocusSet = false;
	};
}

#endif
