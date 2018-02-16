// @file EditorRotationWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_ROTATION_WIDGET_H
#define ENJON_EDITOR_ROTATION_WIDGET_H

#include "EditorWidget.h"

#include <Graphics/Renderable.h> 

namespace Enjon
{ 
	class EditorRotationWidget : public EditorWidget
	{

		public:
			EditorRotationWidget( ) = default;
			~EditorRotationWidget( ) = default; 

			virtual void Initialize( EditorTransformWidget* owner ); 
			virtual void BeginInteraction( TransformWidgetRenderableType type );
			virtual void Interact( );
			virtual void EndInteraction( TransformWidgetRenderableType type ); 
			
			TransformRenderableHeirarchy mRoot;
			TransformRenderableHeirarchy mUpAxis; 
			TransformRenderableHeirarchy mForwardAxis;
			TransformRenderableHeirarchy mRightAxis;
	}; 
}

#endif
