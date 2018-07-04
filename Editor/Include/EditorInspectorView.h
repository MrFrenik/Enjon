// File: EditorInspectorView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_INSPECTOR_VIEW_H
#define ENJON_EDITOR_INSPECTOR_VIEW_H 

#include "EditorView.h" 

namespace Enjon
{ 
	class Entity;

	class EditorInspectorView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorInspectorView( EditorApp* app, Window* window );

			/**
			* @brief
			*/
			~EditorInspectorView( ) = default; 

			/**
			* @brief
			*/
			void SetInspetedObject( const Object* object );

			/**
			* @brief
			*/
			void DeselectInspectedObject( const Object* object );
 
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
			void InspectEntityViewHeader( Entity* ent );

		protected: 

			const Object* mInspectedObject = nullptr;
	};
}

#endif
