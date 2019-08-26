 
#pragma once 
#ifndef ENJON_MATERIAL_EDIT_WINDOW_H
#define ENJON_MATERIAL_EDIT_WINDOW_H

#include "EditorView.h"
#include "EditorTransformWidget.h"

#include <Entity/EntityManager.h>
#include <Graphics/Renderable.h> 
#include <Graphics/Camera.h>
#include <Scene/Scene.h>
#include <Base/World.h> 
#include <Graphics/Window.h>
#include <Asset/Asset.h>
#include <Graphics/StaticMeshRenderable.h>

namespace Enjon
{ 
	class Material;
	class Archetype;
	class EditorWorldOutlinerView;
	class EditorInspectorView;

	ENJON_ENUM( )
	enum class ViewportCallbackType
	{
		AssetDropArchetype,
		AssetDropStaticMesh,
		AssetDropSkeletalMesh,
		CustomRenderOverlay,
	};

	// Pass some user data into the callback
	using ViewportCallback = std::function< void( void* ) >;

	class EditorViewport : public EditorView
	{
		public:
			/**
			* @brief
			*/
			EditorViewport( EditorApp* app, Window* window, const String& name = "Viewport" )
				: EditorView( app, window, name, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
			{ 
			} 

			/**
			* @brief
			*/
			~EditorViewport( ) = default; 

			/**
			* @brief
			*/
			Vec2 GetCenterOfViewport( );	 

			/**
			* @brief
			*/
			Vec2 GetSceneViewProjectedCursorPosition( );

			/**
			* @brief
			*/
			bool HasViewportCallback( const ViewportCallbackType& type );

			/**
			* @brief
			*/
			void SetViewportCallback( ViewportCallbackType type, const ViewportCallback& callback );

			/**
			* @brief
			*/
			void UpdateCamera( );

		protected:

			/**
			* @brief Must be overriden
			*/
			virtual void UpdateView( ) override;

			/**
			* @brief Must be overriden
			*/
			virtual void ProcessViewInput( ) override
			{ 
			}

			/**
			* @brief Must be overriden
			*/
			virtual void Initialize( ) override
			{ 
			}

			/**
			* @brief
			*/
			virtual void CaptureState( ) override;

			/**
			* @brief
			*/
			void HandleAssetDrop( ); 

		protected: 
			Vec2 mSceneViewWindowPosition;
			Vec2 mSceneViewWindowSize; 
			f32 mCameraSpeed = 10.f;
			f32 mMouseSensitivity = 10.0f;
			Transform mPreviousCameraTransform;
			Vec2 mMouseCoordsDelta = Vec2( 0.0f );
			bool mStartedFocusing = false;
			bool mFocusSet = false;
			HashMap< ViewportCallbackType, ViewportCallback > mViewportCallbacks;
	}; 

	class EditorTransformWidgetToolBar : public EditorView
	{
		public:
			/**
			* @brief
			*/
			EditorTransformWidgetToolBar( EditorApp* app, Window* window )
				: EditorView( app, window, "Transform ToolBar", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
			{ 
			}

			/**
			* @brief
			*/
			~EditorTransformWidgetToolBar( ) = default; 

		protected:

			/**
			* @brief Must be overriden
			*/
			virtual void UpdateView( ) override;

			/**
			* @brief Must be overriden
			*/
			virtual void ProcessViewInput( ) override
			{ 
			}

			/**
			* @brief Must be overriden
			*/
			virtual void Initialize( ) override
			{ 
			}

			///**
			//* @brief
			//*/
			//virtual void CaptureState( ) override;
	}; 

	ENJON_CLASS( )
	class EditorMaterialEditWindow : public Window
	{
		ENJON_CLASS_BODY( EditorMaterialEditWindow )

		public: 

			/**
			* @brief
			*/
			EditorMaterialEditWindow( const AssetHandle< Material >& mat );

			virtual void Init( const WindowParams& params ) override; 

		protected:

			virtual void ExplicitDestroy( ) override
			{
				if ( mViewport )
				{
					delete ( mViewport );
					mViewport = nullptr;
				}
			}

			void ConstructScene( ); 

		protected: 
			EditorViewport* mViewport = nullptr;
			AssetHandle< Material > mMaterial;
			StaticMeshRenderable mRenderable;
			b32 mInitialized = false;
			bool mViewportOpen = true;
	}; 

	ENJON_CLASS( )
	class EditorTextureEditWindow : public Window
	{
		ENJON_CLASS_BODY( EditorTextureEditWindow )

		public:

			virtual void Init( const WindowParams& params ) override;

		protected:

			virtual void ExplicitDestroy( ) override
			{
				if ( mViewport )
				{
					delete mViewport;
					mViewport = nullptr;
				} 

				if ( mMaterial )
				{
					delete mMaterial;
					mMaterial = nullptr;
				}
			}

			void ConstructScene( );

		protected:

			EditorViewport* mViewport = nullptr;
			AssetHandle< Texture > mTexture; 
			StaticMeshRenderable mRenderable;
			Material* mMaterial = nullptr;
	};
}

#endif
