// @file EditorRotationWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <Graphics/GraphicsSubsystem.h>
#include <SubsystemCatalog.h>

#include <fmt/format.h>

#include "EditorRotationWidget.h"

namespace Enjon
{
	void EditorRotationWidget::Initialize( EditorTransformWidget* owner )
	{ 
		// Set up renderables for each of the axes
		AssetManager* am = EngineSubsystem( AssetManager );

		// Set up renderables
		mRoot.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_sphere" ) );
		mForwardAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_ring" ) );
		mRightAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_ring" ) );
		mUpAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_ring" ) );
 
		// Set up materials
		mRoot.mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ) );
		mForwardAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "BlueMaterial" ) );
		mRightAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "RedMaterial" ) );
		mUpAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "GreenMaterial" ) ); 

		mRoot.mLocalTransform.SetScale( 0.2f );

		// Child heirarchies
		mRoot.AddChild( &mForwardAxis );
		mRoot.AddChild( &mUpAxis );
		mRoot.AddChild( &mRightAxis );

		// Set up local transforms for each
		f32 yScale = 5.0f;
		f32 xzScale = 10.0f;
		f32 bias = 1.0f;
		f32 coneYOffset = 1.2f;
		Vec3 coneScale = Vec3( 2.0f, 0.3f, 2.0f );
		Vec3 sharedAxisScale = Vec3( 3.0f, 0.4f, 0.2f );
		mForwardAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( -90.0f ), Vec3::XAxis() ) );
		mRightAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( 90.0f ), Vec3::ZAxis() ) );
		mForwardAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mRightAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mUpAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );

		// Set renderable ids 
		mRoot.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationRoot );
		mForwardAxis.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::RotationForwardAxis );
		mUpAxis.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::RotationUpAxis );
		mRightAxis.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::RotationRightAxis );

		// Add transformation hierarchies
		mTransformHeirarchies.push_back( &mRoot );
		mTransformHeirarchies.push_back( &mForwardAxis );
		mTransformHeirarchies.push_back( &mUpAxis );
		mTransformHeirarchies.push_back( &mRightAxis );

		mRoot.mLocalTransform.SetPosition( Vec3( 0.0f, 0.0f, 0.0f ) );

		// Set owner
		mOwner = owner;

		// Set root hierarchy
		mRootHeirarchy = &mRoot;
	} 

	void EditorRotationWidget::BeginInteraction( TransformWidgetRenderableType type )
	{
		AssetManager* am = EngineSubsystem( AssetManager );

		// Change material 
		switch ( type )
		{ 
			case TransformWidgetRenderableType::RotationForwardAxis:
			{
				mForwardAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "YellowMaterial" ) );
			} break;

			case TransformWidgetRenderableType::RotationRightAxis:
			{
				mRightAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "YellowMaterial" ) );
			} break;

			case TransformWidgetRenderableType::RotationUpAxis:
			{
				mUpAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "YellowMaterial" ) );
			} break; 
		}
	}

	void EditorRotationWidget::Interact( )
	{

	}

	void EditorRotationWidget::EndInteraction( TransformWidgetRenderableType type )
	{
		AssetManager* am = EngineSubsystem( AssetManager );

		// Change material 
		switch ( type )
		{
			case TransformWidgetRenderableType::RotationForwardAxis:
			{
				mForwardAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "BlueMaterial" ) );
			} break; 

			case TransformWidgetRenderableType::RotationRightAxis:
			{
				mRightAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "RedMaterial" ) );
			} break;

			case TransformWidgetRenderableType::RotationUpAxis:
			{
				mUpAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "GreenMaterial" ) );
			} break; 
		}
	}
}
