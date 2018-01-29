// @file EditorScaleWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_SCALE_WIDGET_H
#define ENJON_EDITOR_SCALE_WIDGET_H

#include "EditorWidget.h"

#include <Graphics/Renderable.h> 

namespace Enjon
{ 
	class EditorScaleWidget : public EditorWidget
	{ 
		public:
			EditorScaleWidget( ) = default;
			~EditorScaleWidget( ) = default; 

			void Initialize( EditorTransformWidget* owner );

			virtual void BeginInteraction( TransformWidgetRenderableType type );
			virtual void Interact( );
			virtual void EndInteraction( TransformWidgetRenderableType type ); 
			
			TransformRenderableHeirarchy mRoot;
			TransformRenderableHeirarchy mUpAxis;
			TransformRenderableHeirarchy mXYAxis; 
			TransformRenderableHeirarchy mXZAxis; 
			TransformRenderableHeirarchy mYZAxis;
			TransformRenderableHeirarchy mUpAxisArrow;
			TransformRenderableHeirarchy mForwardAxis;
			TransformRenderableHeirarchy mForwardAxisArrow;
			TransformRenderableHeirarchy mRightAxis;
			TransformRenderableHeirarchy mRightAxisArrow;
	}; 
}

#endif
