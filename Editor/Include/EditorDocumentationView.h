// File: EditorDocumentationView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_DOCUMENTATION_VIEW_H
#define ENJON_EDITOR_DOCUMENTATION_VIEW_H 

#include "EditorView.h" 

namespace Enjon
{ 
	class Entity;

	class EditorDocumentationView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorDocumentationView( EditorApp* app, Window* window );

			/**
			* @brief
			*/
			~EditorDocumentationView( ) = default; 

			/**
			* @brief
			*/
			void SetInspetedClassID( const s32& clsID );

			/**
			* @brief
			*/
			void DeselectInspectedCls( );
 
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

			void ViewClass( const MetaClass* cls );

		protected: 

			s32 mInspectedClsID = -1;
	};
}

#endif
