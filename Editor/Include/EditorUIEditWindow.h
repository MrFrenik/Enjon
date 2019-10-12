#pragma once 
#ifndef ENJON_UI_EDIT_WINDOW_H
#define ENJON_UI_EDIT_WINDOW_H

#include "EditorView.h"
#include "EditorTransformWidget.h"

#include <Base/World.h> 
#include <Graphics/Window.h>
#include <Asset/Asset.h>
#include <GUI/UIAsset.h>

namespace Enjon
{
	ENJON_CLASS( Construct )
	class EditorUIEditWindow : public Window
	{
		ENJON_CLASS_BODY( EditorUIEditWindow )

		public:

			virtual void Init( const WindowParams& params ) override;

		protected:

			void ConstructScene();

		protected:

			void Canvas();
			void DrawUI();

			void SelectElement( UIElement* element );
			void DeselectElement();
			ImVec2 HandleMousePan();

		private:

			Transform mViewportTransform = Transform( Vec3( 0.f ), Quaternion(), Vec3( 800.f, 600.f, 1.f ) );
			Transform mCamTransform;

			UIElement* mSelectedElement = nullptr;

			AssetHandle< UI > mUI;
			b32 mInitialized = false;
			b32 mCanvasInitialized = false;
	};
}

#endif
