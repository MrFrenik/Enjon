#pragma once 
#ifndef ENJON_UI_EDIT_WINDOW_H
#define ENJON_UI_EDIT_WINDOW_H

#include "EditorView.h"
#include "EditorTransformWidget.h"

#include <Base/World.h> 
#include <Graphics/Window.h>
#include <Asset/Asset.h>
#include <GUI/UIAsset.h>
#include <Graphics/Camera.h>
#include <Graphics/RenderPass.h>

namespace Enjon
{
	class FrameBuffer;

	ENJON_CLASS( Construct )
	class UIRenderPass : public RenderPass
	{
		ENJON_CLASS_BODY( UIRenderPass )

		public:
			UIRenderPass( const AssetHandle< UI >& ui, const Window* window );

			u32 GetFramebufferTextureId() const;
 
		protected:

			virtual void Render() override;

		private: 
			AssetHandle< UI > mUI; 
			FrameBuffer* mFrameBuffer = nullptr;
			const Window* mWindow = nullptr;
			GUIContext* mGUIContext = nullptr;
	};

	class EditorViewport;

	ENJON_CLASS( Construct )
	class EditorUICanvasWindow : public Window
	{
		ENJON_CLASS_BODY( EditorUICanvasWindow )

		public:

			virtual void Init( const WindowParams& params ) override; 
			virtual void Update() override;

		protected:

			void ConstructScene();

		private:
			b32 mInitialized = false;
			AssetHandle< UI > mUI;
	};

	ENJON_CLASS( Construct )
	class EditorUIEditWindow : public Window
	{
		ENJON_CLASS_BODY( EditorUIEditWindow )

		public:

			virtual void ExplicitDestructor() override;

			virtual void Init( const WindowParams& params ) override;

			virtual void Update() override;

		protected:

			void ConstructScene();

		protected:

			void Canvas();
			void DrawUI();

			void SelectElement( UIElement* element );
			void DeselectElement();
			ImVec2 HandleMousePan();
			bool DrawRectHandle( const ImVec2& center, const u32& id, ImVec2* delta ); 
			void DrawUIElementsRecursive( UIElement* element );

		private:

			s32 mCanvasWindowID = -1;
			Transform mViewportTransform = Transform( Vec3( 0.f ), Quaternion(), Vec3( 800.f, 600.f, 1.f ) );
			Camera mCamera;

			UIElement* mSelectedElement = nullptr;

			UIRenderPass* mUIPass = nullptr;
			AssetHandle< UI > mUI;
			b32 mInitialized = false;
			b32 mCanvasInitialized = false;
	};
}

#endif
