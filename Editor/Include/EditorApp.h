// @file EditorApp.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Project.h"
#include "EditorTransformWidget.h"
#include "EditorView.h"
#include "EditorSceneView.h"
#include "EditorInspectorView.h"
#include "EditorObject.h"

#include <Application.h>
#include <Entity/EntityManager.h>
#include <Graphics/Renderable.h> 
#include <Graphics/Camera.h>
#include <Scene/Scene.h>
#include <Base/World.h> 

namespace Enjon
{ 
	class EditorViewport;
	class EditorWindow;

	class EditorWidgetManager
	{ 
		public: 

			/**
			* @brief
			*/
			EditorWidgetManager( ) = default;

			/**
			* @brief
			*/
			~EditorWidgetManager( )
			{
				// Free memory
				for ( auto& v : mViews )
				{
					delete v;
					v = nullptr;
				}

				// Free all memory from maps
				mFocusedMap.clear( );
				mHoveredMap.clear( );
				mViews.clear( );
				mEditorObjects.clear( );
			} 
 
			/**
			* @brief
			*/ 
			bool HasView( EditorView* view )
			{
				return ( std::find( mViews.begin( ), mViews.end( ), view ) != mViews.end( ) );
			}

			/**
			* @brief
			*/
			EditorView* AddView( EditorView* view )
			{ 
				if ( !HasView( view ) )
				{
					mFocusedMap[ view ] = false;
					mHoveredMap[ view ] = false;
					mViewEnabledMap[ view ] = view->GetEnabled();
					mViews.push_back( view );

					// Initialize view
					view->Initialize( );
				}

				// Return the view from the function
				return view;
			}

			/**
			* @brief
			*/
			void SetHovered( EditorObject* object, bool hovered )
			{ 
				mHoveredMap[ object ] = hovered;
			}

			/**
			* @brief
			*/
			void SetFocused( EditorObject* object, bool focused )
			{
				mFocusedMap[ object ] = focused;
			} 

			/**
			* @brief
			*/
			bool GetHovered( EditorObject* object )
			{
				if ( HasHoveredObject( object ) )
				{
					return mHoveredMap[ object ];
				}

				return false;
			}

			/**
			* @brief
			*/
			bool GetFocused( EditorObject* object )
			{
				if ( HasFocusedObject( object ) )
				{
					return mFocusedMap[ object ];
				}

				return false;
			}

		private:

			bool HasHoveredObject( EditorObject* object )
			{
				return mHoveredMap.find( object ) != mHoveredMap.end( );
			}

			bool HasFocusedObject( EditorObject* object )
			{
				return mFocusedMap.find( object ) != mFocusedMap.end( );
			}

		private: 

			HashMap< EditorObject*, bool > mFocusedMap;
			HashMap< EditorObject*, bool > mHoveredMap; 
			HashMap< EditorView*, bool > mViewEnabledMap;

			Vector< EditorObject* > mEditorObjects;
			Vector< EditorView* > mViews;
	};

	class EditorApp : public Enjon::Application
	{
		public:

			virtual Enjon::Result Initialize() override;  

			/**
			* @brief Main update tick for application.
			* @return Enjon::Result
			*/
			virtual Enjon::Result Update(f32 dt) override;

			/**
			* @brief Processes input from input class 
			* @return Enjon::Result
			*/
			virtual Enjon::Result ProcessInput(f32 dt) override;

			/**
			* @brief Shuts down application and cleans up any memory that was allocated.
			* @return Enjon::Result
			*/
			virtual Enjon::Result Shutdown() override; 

			/**
			* @brief 
			*/
			Vec2 GetSceneViewProjectedCursorPosition( );

			/**
			* @brief 
			*/
			EditorWidgetManager* GetEditorWidgetManager( );

			/**
			* @brief 
			*/
			void SetEditorSceneView( EditorSceneView* view ); 

			/**
			* @brief 
			*/
			EditorInspectorView* GetInspectorView( );

			/**
			* @brief 
			*/
			String GetBuildConfig( ) const;
 
			/**
			* @brief 
			*/
			String GetVisualStudioDirectoryPath( ) const;

			String GetProjectMainTemplate( ) const;

			String GetCompileProjectCMakeTemplate( ) const;

			String GetBuildAndRunCompileTemplate( ) const;

			String GetProjectEnjonDefinesTemplate( ) const;

			Project* GetProject( );

			EntityHandle GetSelectedEntity( );

			void SelectEntity( const EntityHandle& handle );

			void DeselectEntity( );

			void EnableOpenNewComponentDialogue( );
			void OpenNewComponentDialogue( ); 
			void AddComponentPopupView( );

		public:
			Vec4 mRectColor = Vec4( 0.8f, 0.3f, 0.1f, 1.0f );

		private:
			void CreateComponent( const String& componentName );
			void LoadResourceFromFile( );
			void PlayOptions( );
			void CameraOptions( bool* enable );
			void CreateProjectView( );
			void SelectSceneView( );

			void LoadProjectView( );
			void CheckForPopups( );

			void CreateNewProject( const String& projectName );


			void LoadProject( const Project& project );
			bool UnloadDLL( ByteBuffer* buffer = nullptr );
			void LoadDLL( bool releaseSceneAsset = true );
			void ReloadDLL( );

			void CollectAllProjectsOnDisk( );
			void LoadProjectSolution( );

			void CleanupScene( );

			void InitializeProjectApp( );
			void ShutdownProjectApp( ByteBuffer* buffer );

			void LoadResources( ); 
			void LoadProjectResources( ); 

			void InspectorView( bool* enabled );
			void SceneView( bool* viewBool ); 

			void TestSecondWindow( );

		private:
			bool mViewBool = true;
			bool mShowCameraOptions = true;
			bool mShowLoadResourceOption = true;
			bool mShowCreateProjectView = true;
			bool mShowSceneView = true;
			Enjon::String mResourceFilePathToLoad = "";
			bool mMoveCamera = false; 
			bool mNewComponentPopupDialogue = false;
			bool mLoadProjectPopupDialogue = false;

			bool mPlaying = false;
			bool mNeedsStartup = true; 
			bool mNeedsShutdown = false;

			Enjon::f32 mCameraSpeed = 10.f;
			Enjon::f32 mMouseSensitivity = 10.0f;
			Enjon::Transform mPreviousCameraTransform;

			Project mProject;

			String mNewProjectName = "";
 
			String mProjectsPath = "";
			String mProjectSourceTemplate = "";
			String mProjectCMakeTemplate = "";
			String mProjectDelBatTemplate = "";
			String mProjectBuildAndRunTemplate = "";
			String mProjectBuildAndRunCompileTemplate = "";
			String mComponentSourceTemplate = "";
			String mCompileProjectBatTemplate = "";
			String mCompileProjectCMakeTemplate = "";
			String mProjectMainTemplate = "";
			String mProjectEnjonDefinesTemplate = "";
			String mProjectBuildBatTemplate = "";

			Vector<Project> mProjectsOnDisk;
			Vector<Entity*> mSceneEntities;

			EntityHandle mSceneEntity; 

			EditorTransformWidget mTransformWidget;

			Vec2 mSceneViewWindowPosition;
			Vec2 mSceneViewWindowSize;
			EntityHandle mSelectedEntity;

			// This could get dangerous...
			AssetHandle<Scene> mCurrentScene;

			EditorWidgetManager mEditorWidgetManager;

			EditorSceneView* mEditorSceneView = nullptr;

			EditorInspectorView* mInspectorView = nullptr;

			Camera mEditorCamera;
			Vec3 mCameraRotator = Vec3( 0.0f );

			EditorWindow* mTestEditorWindow = nullptr;
	}; 
}

