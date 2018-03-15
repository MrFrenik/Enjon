// File: EditorInspectorView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_INSPECTOR_VIEW_H
#define ENJON_EDITOR_INSPECTOR_VIEW_H 

#include "EditorView.h" 

namespace Enjon
{ 
	class EditorInspectorView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorInspectorView( EditorApp* app );

			/**
			* @brief
			*/
			~EditorInspectorView( ) = default; 
 
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

		protected: 
	};
}

#endif
