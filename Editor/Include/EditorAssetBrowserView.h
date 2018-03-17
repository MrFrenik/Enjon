// File: EditorAssetBrowserView.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_ASSET_BROWSER_VIEW_H
#define ENJON_EDITOR_ASSET_BROWSER_VIEW_H 

#include "EditorView.h"

#include <ImGui/ImGuiManager.h>

#include <Math/Vec2.h>

namespace Enjon
{
	class Project; 

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
			* @brief Must be overridden
			*/
			virtual void UpdateView( ) override;

			/**
			* @brief Must be overridden
			*/
			virtual void ProcessViewInput( ) override;

			/**
			* @brief Must be overridden
			*/
			virtual void Initialize( ) override; 

			void InitializeCurrentDirectory( Project* project );

			void AttemptDirectoryBackTraversal( );

			void SetSelectedPath( const String& path );

			void FolderMenuPopup( );

			void NewFolderMenuOption( );

			void CreateMenuOption( );

		protected: 
			String mCurrentDirectory = "";
			String mRootDirectory = "";
			String mSelectedPath = "";

			PopupWindow mFolderMenuPopup;
	};
}

#endif
