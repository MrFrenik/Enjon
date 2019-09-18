// @file EditorApp.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once 
#ifndef ENJON_EDITOR_H
#define ENJON_EDITOR_H

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

	/*
		// What does a tool chain look like? 
		- Environment: 
			- The actual tool chain 'heart'. For Windows platform, this could be: 
				- MSVC
				- Cygwin
				- MinGW
				- WSL
		- For Visual Studio: 
			- Need to detect the version to be called with CMake
			- Need to detect location of MSBuild (or NMake)
			- Need to find c++ compiler location (which is MSBuild, right?)
			- Could use vswhere to detect locations for certain packages (or could attempt to detect things myself, 
					using standard locations). If these things cannot be detected, then you should 
					set them up yourself.
		- The editor should hold a hashmap of tool chains that are serialized with its configuration information
		- Users can create/edit/delete tool chains from within the editor
		- It's totally possible to mix/match as well. For instance, what if the user wants to use visual studio, but 
			the compiler they've chosen is mingw? This should be possible. Or they want to compile via NMake? This is getting 
			complicated. Maybe CLion's set up is TOO restrictive? Perhaps I should instead go with something like 
			VSCode's tasks? 
		- What about per project setup? Such as additional include directories / library directories? Linking libraries? Dynamic and static
			libs? 
	*/

	// Is this a fair distinction? I suppose for now?  
	ENJON_ENUM()
	enum class ToolChainEnvironment
	{
		MSVC,
		Count
	};

	// Want the tool chain to attempt to auto-detect libraries

	ENJON_CLASS( Abstract )
	class ToolChain : public Object
	{ 
		ENJON_CLASS_BODY( ToolChain )
			
		public: 

			virtual Result FindPaths() {
				return Result::INCOMPLETE;
			}

			virtual Result FindCompilerPath() {
				return Result::INCOMPLETE;
			} 

			virtual Result FindMakePath() {
				return Result::INCOMPLETE;
			} 

			virtual b32 IsValid() = 0;

			virtual Result Compile() = 0;

		public:
			ENJON_PROPERTY( )
			ToolChainEnvironment mEnvironment;

			ENJON_PROPERTY()
			String mCompilerDirectory = "invalid";

			ENJON_PROPERTY()
			String mMakeDirectory = "invalid";

			ENJON_PROPERTY()
			String mCmakeDirectory = "invalid";

			ENJON_PROPERTY()
			String mCMakeFlags = "invalid";

			ENJON_PROPERTY()
			String mName = "invalid";
	};

	ENJON_ENUM()
	enum class VisualStudioVersion
	{ 
		VS2013,
		VS2015, 
		VS2017, 
		VS2019,
		Count
	};

	// Want to be able to serialize/deserialize .json files... eventually 

	ENJON_CLASS( Construct )
	class ToolChainMSVC : public ToolChain
	{ 
		ENJON_CLASS_BODY( ToolChainMSVC )
			
		public:

			virtual Result FindPaths() override;
		
			virtual Result FindCompilerPath() override;

			// I guess? This just seems odd though...
			Result FindVisualStudioPath();

			virtual Result OnEditorUI() override; 

			virtual b32 IsValid() override;
			
			virtual Result Compile() override {
				return Result::INCOMPLETE;
			}

			Result FindMSBuildPath(); 

		public: 
			ENJON_PROPERTY()
			String mVisualStudioDirectory = "invalid"; 

			ENJON_PROPERTY()
			VisualStudioVersion mVSVersion = VisualStudioVersion::VS2015;
	}; 

	ENJON_CLASS( Construct )
	class EditorConfigSettings : public Object
	{
		ENJON_CLASS_BODY( EditorConfigSettings )

		/*
			Config settings need: 
				- Build system information:
					- Name
					- Compiler Directory
					- CMake Flags
				- Recent project list ( manifest )
					- Project names
					- Project directories 
				- Various editor settings for user config
		*/	

		public:

			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* archiver ) const override;
			
			/*
			* @brief //
			*/
			virtual Result DeserializeData( ByteBuffer* archiver ) override; 

		public: 
			ENJON_PROPERTY( NonSerializeable )
			Vector< Project > mProjectList;

			ENJON_PROPERTY( NonSerializeable )
			ToolChain* mToolChains[ (u32)ToolChainEnvironment::Count ];

			ENJON_PROPERTY()
			ToolChainEnvironment mToolChainID = ToolChainEnvironment::MSVC;

			ENJON_PROPERTY()
			String mLastUsedProjectDirectory = "";

			ENJON_PROPERTY( NonSerializeable )
			Vector< ToolChainDefinition > mToolChainDefinitions;

			EditorApp* mEditor = nullptr;
	};

	// TODO(john): Need to reflect over the editor app to get introspection meta data
	ENJON_CLASS( Construct )
	class EditorApp : public Application
	{
		ENJON_MODULE_BODY( EditorApp )

		public:

			virtual Result Initialize() override;  

			/**
			* @brief Main update tick for application.
			* @return Result
			*/
			virtual Result Update(f32 dt) override;

			/**
			* @brief Processes input from input class 
			* @return Result
			*/
			virtual Result ProcessInput(f32 dt) override;

			/**
			* @brief Shuts down application and cleans up any memory that was allocated.
			* @return Enjon::Result
			*/
			virtual Result Shutdown() override; 

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

			EditorTransformWidget* GetTransformWidget( );

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
			void ProjectListView( ); 
			void NewProjectView( );

			void LoadProjectView( );
			void CheckForPopups( );

			void CreateNewProject( const ProjectConfig& projectName );


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
			void UnloadPreviousProject( );
			void FindBuildSystem( );

			void PreloadProject( const Project& project );

			void CleanupGUIContext( );

			void PreCreateNewProject( const ProjectConfig& projectName );

			void FindProjectOnLoad( );

			void DeserializeEditorConfigSettings( ); 
			void SerializeEditorConfigSettings( );

			void ToolChainView( );
			
			void InitializeToolChains( );

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
			bool mNeedRecompile = false;
			bool mNeedReload = false;

			Enjon::f32 mCameraSpeed = 10.f;
			Enjon::f32 mMouseSensitivity = 10.0f;
			Enjon::Transform mPreviousCameraTransform;

			Project mProject;

			String mNewProjectName = "NewProject";
			ProjectConfig mNewProjectConfig;
 
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

			ENJON_PROPERTY( HideInEditor )
			EditorConfigSettings mConfigSettings; 
	}; 

	// Declaration for module export
	ENJON_MODULE_DECLARE( EditorApp ) 

}

#endif
