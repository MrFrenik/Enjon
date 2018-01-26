// @file EditorTranslationWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_TRANSLATION_WIDGET_H
#define ENJON_EDITOR_TRANSLATION_WIDGET_H

#include "EditorWidget.h"

#include <Graphics/Renderable.h> 

namespace Enjon
{ 
	ENJON_CLASS()
	class EditorTranslationWidget : public Object
	{
		ENJON_CLASS_BODY( )

		public:
			EditorTranslationWidget( ) = default;
			~EditorTranslationWidget( ) = default; 

			void Initialize( );

			void Update( );

			Transform GetWorldTransform( );

			// Sets root position
			void SetPosition( const Vec3& position );
			// Sets root scale
			void SetScale( const f32& scale );
			
			TransformRenderableHeirarchy mRoot;
			TransformRenderableHeirarchy mUpAxis;

			ENJON_PROPERTY( )
			TransformRenderableHeirarchy mXYAxis;

			ENJON_PROPERTY( )
			TransformRenderableHeirarchy mXZAxis;

			ENJON_PROPERTY( )
			TransformRenderableHeirarchy mYZAxis;

			TransformRenderableHeirarchy mUpAxisArrow;
			TransformRenderableHeirarchy mForwardAxis;
			TransformRenderableHeirarchy mForwardAxisArrow;
			TransformRenderableHeirarchy mRightAxis;
			TransformRenderableHeirarchy mRightAxisArrow;
	}; 
}

#endif
