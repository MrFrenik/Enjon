
#include "EditorView.h"

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

	class EditorViewport : public EditorView
	{
		public:
			/**
			* @brief
			*/
			EditorViewport( EditorApp* app, Window* window )
				: EditorView( app, window, "Viewport", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse )
			{ 
			}

			/**
			* @brief
			*/
			~EditorViewport( ) = default; 

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
			virtual void CaptureState( )
			{ 
			} 

		protected:
			Vec2 mSceneViewWindowPosition;
			Vec2 mSceneViewWindowSize; 
	}; 

	class EditorMaterialEditWindow : public Window
	{
		public: 

			/**
			* @brief
			*/
			EditorMaterialEditWindow( const AssetHandle< Material >& mat );

			virtual int Init( std::string windowName, int screenWidth, int screenHeight, WindowFlagsMask currentFlags = WindowFlagsMask( ( u32 )WindowFlags::RESIZABLE ) ) override; 

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
			u32 mInitialized : 1;
			bool mViewportOpen = true;
	};

}
