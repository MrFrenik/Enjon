// @file EditorTranslationWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_TRANSLATION_WIDGET_H
#define ENJON_EDITOR_TRANSLATION_WIDGET_H

#include "EditorWidget.h"

#include <Graphics/Renderable.h> 

namespace Enjon
{ 
	class EditorTranslationWidget
	{
		public:
			EditorTranslationWidget( ) = default;
			~EditorTranslationWidget( ) = default; 

			void Initialize( );

			void Update( );

			Transform GetWorldTransform( );

			// Sets root position
			void SetPosition( const Vec3& position );
			
			TransformHeirarchy mRoot;
			TransformRenderableHeirarchy mUpAxis;
			TransformRenderableHeirarchy mForwardAxis;
			TransformRenderableHeirarchy mRightAxis;
	}; 
}

#endif
