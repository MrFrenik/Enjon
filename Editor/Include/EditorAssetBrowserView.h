// File: EditorAssetBrowserView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_ASSET_BROWSER_VIEW_H
#define ENJON_EDITOR_ASSET_BROWSER_VIEW_H 

#include "EditorView.h"

#include <Math/Vec2.h>

namespace Enjon
{
	class Project;

	struct ContextMenu
	{
		void Activate( const Vec2& coords )
		{
			mOpened = true;
			mOpenPosition = coords;
		}

		Vec2 GetPosition( )
		{
			return mOpenPosition;
		}

		Vec2 GetSize( )
		{
			return mSize;
		}

		void Deactivate( )
		{
			mOpened = false;
		}

		bool IsActive( )
		{
			return mOpened;
		}

		bool mOpened = false;
		Vec2 mOpenPosition; 
		Vec2 mSize = Vec2( 200.0f, 300.0f );
	};

	class EditorAssetBrowserView : public EditorView
	{
		public:

			/**
			* @brief
			*/
			EditorAssetBrowserView( EditorApp* app );

			/**
			* @brief
			*/
			~EditorAssetBrowserView( ) = default; 
 
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
			void ContextMenuInteraction( ContextMenu* menu );

			void InitializeCurrentDirectory( Project* project );

			void AttemptDirectoryBackTraversal( );

		protected: 
			ContextMenu mContextMenu;
			String mCurrentDirectory = "";
			String mRootDirectory = "";
	};
}

#endif
