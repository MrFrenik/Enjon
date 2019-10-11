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

		private:
			AssetHandle< UI > mUI;
			b32 mInitialized = false;
	};
}

#endif
