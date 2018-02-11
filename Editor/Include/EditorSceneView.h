// File: EditorSceneView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_SCENE_VIEW_H
#define ENJON_EDITOR_SCENE_VIEW_H 

#include "EditorView.h"

#include <Math/Vec2.h>

namespace Enjon
{
	class EditorSceneView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorSceneView( EditorApp* app );

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

		protected: 
			Vec2 mSceneViewWindowPosition;
			Vec2 mSceneViewWindowSize; 
	};
}

#endif
