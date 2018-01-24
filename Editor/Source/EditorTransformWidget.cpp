// @file EditorTransformWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "EditorTransformWidget.h"

namespace Enjon
{
	void EditorTransformWidget::Initialize( )
	{
		// Initialize the translation widget
		mTranslationWidget.Initialize( );
	}

	void EditorTransformWidget::Update( )
	{
		mTranslationWidget.Update( );
	}

	Transform EditorTransformWidget::GetWorldTransform( )
	{
		return mTranslationWidget.GetWorldTransform( );
	}

	void EditorTransformWidget::SetPosition( const Vec3& position )
	{
		mTranslationWidget.SetPosition( position );
	}
}
