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
	using ReloadDLLCallback = std::function< void( void ) >;

	class StaticMeshRenderable;
	class EditorAssetBrowserView;
	class EditorViewport;
	class EditorWindow;
	class EditorMaterialEditWindow; 
	class EditorWorldOutlinerView;
	class EditorArchetypeEditWindow; 
	class EditorTransformWidgetToolBar;

	enum class TransformMode
	{
		Translate,
		Scale,
		Rotate
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
			void RegisterReloadDLLCallback( const ReloadDLLCallback& callback ); 

			/**
			* @brief 
			*/
			Vec2 GetSceneViewProjectedCursorPosition( ); 

			/**
			* @brief 
			*/
			EditorInspectorView* GetInspectorView( );

			/**
			* @brief 
			*/
			EditorAssetBrowserView* GetEditorAssetBrowserView( );

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

			void SetProjectOnLoad( const String& projectDir );

			void EnableTransformSnapping( bool enable, const TransformationMode& mode );
			bool IsTransformSnappingEnabled( const TransformationMode& mode );

			f32 GetTransformSnap( const TransformationMode& mode );
			void SetTransformSnap( const TransformationMode& mode, const f32& val );

		public:
			Vec4 mRectColor = Vec4( 0.8f, 0.3f, 0.1f, 1.0f );

		private:
			void CreateComponent( const String& componentName );
			void LoadResourceFromFile( );
			void PlayOptions( );
			void CameraOptions( bool* enable );
			bool CreateProjectView( );
			void SelectSceneView( );
			void SelectProjectDirectoryView( );

			void LoadProjectView( );
			void CheckForPopups( );

			void CreateNewProject( const String& projectName );


			void LoadProject( const Project& project );
			bool UnloadDLL( ByteBuffer* buffer = nullptr );
			void LoadDLL( bool releaseSceneAsset = true );
			void ReloadDLL( );

			void WriteEditorConfigFileToDisk( );
			void CollectAllProjectsOnDisk( );
			void LoadProjectSolution( );

			void CleanupScene( );

			void InitializeProjectApp( );
			void ShutdownProjectApp( ByteBuffer* buffer );

			void LoadResources( ); 
			void LoadProjectResources( ); 

			void InspectorView( bool* enabled );
			void SceneView( bool* viewBool ); 

			void LoadProjectContext( );
			void LoadProjectSelectionContext( );

			void PreloadProject( const Project& project );

			void CleanupGUIContext( );

			void PreCreateNewProject( const String& projectName );

			void FindProjectOnLoad( );

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
			bool mPreloadProjectContext = false;
			bool mPrecreateNewProject = false; 

			bool mPlaying = false;
			bool mNeedsStartup = true; 
			bool mNeedsShutdown = false;
			bool mNeedsLoadProject = false;

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

			String mProjectOnLoad = "";

			Vector<Project> mProjectsOnDisk;
			Vector<Entity*> mSceneEntities;

			EntityHandle mSceneEntity; 

			EditorTransformWidget mTransformWidget;

			Vec2 mSceneViewWindowPosition;
			Vec2 mSceneViewWindowSize;
			EntityHandle mSelectedEntity;

			// This could get dangerous...
			AssetHandle<Scene> mCurrentScene; 

			EditorViewport* mEditorSceneView = nullptr;
			//EditorSceneView* mEditorSceneView = nullptr; 
			EditorInspectorView* mInspectorView = nullptr; 
			EditorAssetBrowserView* mAssetBroswerView = nullptr; 
			EditorWorldOutlinerView* mWorldOutlinerView = nullptr; 
			EditorArchetypeEditWindow* mArchetypeWindow = nullptr;
			EditorTransformWidgetToolBar* mTransformToolBar = nullptr;

			Camera mEditorCamera;
			Vec3 mCameraRotator = Vec3( 0.0f ); 

			Vector< ReloadDLLCallback > mReloadDLLCallbacks;

			Window* mProjectSelectionWindow = nullptr;
	}; 
}

