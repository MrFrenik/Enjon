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
			// Sets root scale
			void SetScale( const f32& scale );
			void SetRotation( const Quaternion& rotation );

			void Activate( TransformWidgetRenderableType type );
			void Deactivate( TransformWidgetRenderableType type );
			
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
