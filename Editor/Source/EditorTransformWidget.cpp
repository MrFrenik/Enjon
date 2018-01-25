// @file EditorTransformWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "EditorTransformWidget.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Engine.h>
#include <SubsystemCatalog.h>

namespace Enjon
{
	void EditorTransformWidget::Initialize( )
	{
		// Initialize the translation widget
		mTranslationWidget.Initialize( );
	}

	void EditorTransformWidget::Update( )
	{ 
		// Update translation widget
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

	void EditorTransformWidget::SetScale( const f32& scale )
	{
		mTranslationWidget.SetScale( scale );
	}
}
