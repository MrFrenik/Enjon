// @file EditorTransformWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_TRANSFORM_WIDGET_H
#define ENJON_EDITOR_TRANSFORM_WIDGET_H
 
#include "EditorWidget.h"
#include "EditorTranslationWidget.h"

#include <Graphics/Renderable.h>

namespace Enjon
{ 
	class EditorTransformWidget
	{ 
		public:
			EditorTransformWidget( ) = default;
			~EditorTransformWidget( ) = default;

			void Initialize( );
			void Update( );

			Transform GetWorldTransform( );

			void SetPosition( const Vec3& position );
			void SetScale( const f32& scale );
			void SetRotation( const Quaternion& rotation );

			EditorTranslationWidget mTranslationWidget;

			void BeginWidgetInteraction( TransformWidgetRenderableType type );
			void InteractWithWidget( );
			void EndInteraction( );
			TransformWidgetRenderableType GetInteractedWidgetType( );

			bool IsInteractingWithWidget( ) const;
			Vec3 GetDelta( ) const; 

		private:
			Transform mWorldTransform;					
			// Store position and then store that we're moving

			bool mInteractingWithTransformWidget = true; 
			TransformWidgetRenderableType mType = TransformWidgetRenderableType::TranslationRightAxis;
			Vec3 mIntersectionStartPosition;
			Vec3 mRootStartPosition;
			Vec3 mDelta;
	}; 
}

#endif
