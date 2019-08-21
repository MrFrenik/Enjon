
#pragma once 
#ifndef ENJON_ARCHETYPE_EDIT_WINDOW
#define ENJON_ARCHETYPE_EDIT_WINDOW

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

	ENJON_CLASS( )
	class EditorArchetypeEditWindow : public Window
	{ 
		ENJON_CLASS_BODY( EditorArchetypeEditWindow )

		public:

			/**
			* @brief
			*/
			EditorArchetypeEditWindow( const AssetHandle< Archetype >& archetype );

			virtual void Init( const WindowParams& params ) override;

		protected:

			virtual void ExplicitDestroy( ) override;

			void ConstructScene( );

		protected:
			AssetHandle< Archetype > mArchetype;
			EditorWorldOutlinerView* mWorldOutlinerView = nullptr;
			EditorViewport* mViewport = nullptr;
			EditorInspectorView* mInspectorView = nullptr;
			b32 mInitialized = false;
			bool mViewportOpen = true;
			EntityHandle mRootEntity;
			EditorTransformWidget mTransformWidget;
	}; 
}

#endif
