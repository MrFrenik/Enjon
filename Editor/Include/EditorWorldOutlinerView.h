// File: EditorWorldOutlinerView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_WORLD_OUTLINER_VIEW_H
#define ENJON_EDITOR_WORLD_OUTLINER_VIEW_H 

#include "EditorView.h"

#include <Math/Vec2.h>
#include <Entity/EntityManager.h>

namespace Enjon
{
	class EditorWorldOutlinerView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorWorldOutlinerView( EditorApp* app, Window* window );

			/**
			* @brief
			*/
			~EditorWorldOutlinerView( ) = default; 
 
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

		private:

			bool DisplayEntityRecursively( const EntityHandle& handle, u32* entityNumber, const u32& indentionLevel = 0 );

			Vec2 mHeldMousePosition = Vec2( -1, -1 );
			EntityHandle mGrabbedEntity = EntityHandle::Invalid( );
			u32 mEntityNameChangeID = EntityHandle::Invalid( ).GetID( );
	};
}

#endif
