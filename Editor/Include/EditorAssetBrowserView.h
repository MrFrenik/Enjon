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
	class Asset;
	class AssetLoader;

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

			void FolderOptionsMenuPopup( );

			void NewFolderMenuOption( );

			void CreateMenuOption( );

			void PushActivePopupWindow( PopupWindow* window );

			bool ActivePopupWindowEnabled( );

			void ProcessFileDrops( ); 

		protected: 
			String mCurrentDirectory = "";
			String mRootDirectory = "";
			String mSelectedPath = "";
			String mRightClickedPath = "";
			bool mPathNeedsRename = false;

			PopupWindow mFolderMenuPopup;
			PopupWindow mFolderOptionsMenuPopup;

			PopupWindow* mActivePopupWindow = nullptr;
			const Asset* mSelectedAsset = nullptr;
			HashSet<String> mFilesToImport;
			AssetLoader* mCurrentAssetLoader = nullptr;
	};
}

#endif
