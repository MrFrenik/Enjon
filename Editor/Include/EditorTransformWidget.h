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

		private:

			EditorTranslationWidget mTranslationWidget;

		private:
			Transform mWorldTransform;
	}; 
}

#endif
