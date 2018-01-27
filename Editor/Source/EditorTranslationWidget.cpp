// @file EditorTranslationWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <Graphics/GraphicsSubsystem.h>
#include <SubsystemCatalog.h>

#include <fmt/format.h>

#include "EditorTranslationWidget.h"

namespace Enjon
{
	void EditorTranslationWidget::Initialize( )
	{ 
		// Set up renderables for each of the axes
		AssetManager* am = EngineSubsystem( AssetManager );

		// Set up renderables
		mRoot.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_sphere" ) );
		mForwardAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cylinder" ) );
		mForwardAxisArrow.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cone" ) );
		mRightAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cylinder" ) );
		mRightAxisArrow.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cone" ) );
		mUpAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cylinder" ) );
		mUpAxisArrow.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cone" ) );
		mXYAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
		mXZAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
		mYZAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cube" ) );
 
		// Set up materials
		mRoot.mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ) );
		mForwardAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "BlueMaterial" ) );
		mRightAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "RedMaterial" ) );
		mUpAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "GreenMaterial" ) ); 
		mForwardAxisArrow.mRenderable.SetMaterial( am->GetAsset< Material >( "BlueMaterial" ) );
		mUpAxisArrow.mRenderable.SetMaterial( am->GetAsset< Material >( "GreenMaterial" ) );
		mRightAxisArrow.mRenderable.SetMaterial( am->GetAsset< Material >( "RedMaterial" ) );

		mXYAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "BlueMaterial" ) );
		mXZAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "GreenMaterial" ) );
		mYZAxis.mRenderable.SetMaterial( am->GetAsset< Material >( "RedMaterial" ) );

		// Child heirarchies
		mRoot.AddChild( &mForwardAxis );
		mRoot.AddChild( &mUpAxis );
		mRoot.AddChild( &mRightAxis );
		mForwardAxis.AddChild( &mForwardAxisArrow );
		mUpAxis.AddChild( &mUpAxisArrow );
		mRightAxis.AddChild( &mRightAxisArrow ); 
		mUpAxis.AddChild( &mYZAxis );
		mUpAxis.AddChild( &mXYAxis );
		mForwardAxis.AddChild( &mXZAxis );

		// Set up local transforms for each
		f32 yScale = 4.0f;
		f32 xzScale = 0.5f;
		f32 bias = 1.0f;
		f32 coneYOffset = 1.2f;
		Vec3 coneScale = Vec3( 2.0f, 0.3f, 2.0f );
		Vec3 sharedAxisScale = Vec3( 3.0f, 0.4f, 0.2f );
		mForwardAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( -90.0f ), Vec3::XAxis() ) );
		mRightAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( 90.0f ), Vec3::ZAxis() ) );
		mForwardAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mRightAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mUpAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mForwardAxis.mLocalTransform.SetPosition( Vec3( 0.0f, 0.0f, yScale + bias ) );
		mRightAxis.mLocalTransform.SetPosition( Vec3( yScale + bias, 0.0f, 0.0f ) );
		mUpAxis.mLocalTransform.SetPosition( Vec3( 0.0f, yScale + bias, 0.0f ) );
		mForwardAxisArrow.mLocalTransform.SetPosition( Vec3( 0.0f, coneYOffset, 0.0f ) );
		mRightAxisArrow.mLocalTransform.SetPosition( Vec3( 0.0f, coneYOffset, 0.0f ) );
		mUpAxisArrow.mLocalTransform.SetPosition( Vec3( 0.0f, coneYOffset, 0.0f ) );
		mForwardAxisArrow.mLocalTransform.SetScale( coneScale );
		mUpAxisArrow.mLocalTransform.SetScale( coneScale );
		mRightAxisArrow.mLocalTransform.SetScale( coneScale ); 

		mXZAxis.mLocalTransform.SetScale( sharedAxisScale );
		mXZAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( 90.0f ), Vec3::ZAxis( ) ) );
		mXZAxis.mLocalTransform.SetPosition( Vec3( 9.0f, -0.2f, 0.0f ) );

		mXYAxis.mLocalTransform.SetScale( sharedAxisScale );
		mXYAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( 0.0f ), Vec3::YAxis( ) ) );
		mXYAxis.mLocalTransform.SetPosition( Vec3( 9.0f, 0.0f, 0.0f ) ); 

		mYZAxis.mLocalTransform.SetScale( sharedAxisScale );
		mYZAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( -90.0f ), Vec3::YAxis( ) ) );
		mYZAxis.mLocalTransform.SetPosition( Vec3( 0.0f, 0.0f, 9.0f ) );

		// Add renderables to graphics scene
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mRoot.mRenderable );
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mForwardAxis.mRenderable );
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mRightAxis.mRenderable );
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mUpAxis.mRenderable ); 
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mForwardAxisArrow.mRenderable ); 
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mRightAxisArrow.mRenderable ); 
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mUpAxisArrow.mRenderable ); 
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mYZAxis.mRenderable );
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mXYAxis.mRenderable );
		gfx->GetScene( )->AddNonDepthTestedRenderable( &mXZAxis.mRenderable );

		// Set renderable ids 
		mRoot.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationRoot );
		mForwardAxis.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationForwardAxis );
		mUpAxis.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationUpAxis );
		mRightAxis.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationRightAxis );
		mForwardAxisArrow.mRenderable.SetRenderableID( MAX_ENTITIES + ( u32 )TransformWidgetRenderableType::TranslationForwardAxis );
		mUpAxisArrow.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationUpAxis );
		mRightAxisArrow.mRenderable.SetRenderableID( MAX_ENTITIES + (u32)TransformWidgetRenderableType::TranslationRightAxis );
		mYZAxis.mRenderable.SetRenderableID( MAX_ENTITIES + ( u32 )TransformWidgetRenderableType::TranslationYZAxes );
		mXYAxis.mRenderable.SetRenderableID( MAX_ENTITIES + ( u32 )TransformWidgetRenderableType::TranslationXYAxes );
		mXZAxis.mRenderable.SetRenderableID( MAX_ENTITIES + ( u32 )TransformWidgetRenderableType::TranslationXZAxes );

		// Set up scale for root
		mRoot.mLocalTransform.SetScale( 0.5f );

		mRoot.mLocalTransform.SetPosition( Vec3( 0.0f, 0.0f, 0.0f ) );
	} 

	void EditorTranslationWidget::Update( )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		const Camera* cam = gfx->GetSceneCamera( );

		// Set scale of root based on distance from camera
		f32 dist = Vec3::Distance( cam->GetPosition( ), mRoot.mLocalTransform.GetPosition() );
		f32 scale = Clamp( dist / 60.0f, 0.001f, 100.0f );
		mRoot.mLocalTransform.SetScale( scale ); 

		// Calculate world transforms for all
		mRoot.CalculateWorldTransform( ); 
		mForwardAxis.CalculateWorldTransform( );
		mRightAxis.CalculateWorldTransform( );
		mUpAxis.CalculateWorldTransform( );
		mForwardAxisArrow.CalculateWorldTransform( );
		mRightAxisArrow.CalculateWorldTransform( );
		mUpAxisArrow.CalculateWorldTransform( );
		mYZAxis.CalculateWorldTransform( );
		mXYAxis.CalculateWorldTransform( );
		mXZAxis.CalculateWorldTransform( );

		// Set up transforms for renderables
		mRoot.mRenderable.SetTransform( mRoot.mWorldTransform );
		mForwardAxis.mRenderable.SetTransform( mForwardAxis.mWorldTransform );
		mRightAxis.mRenderable.SetTransform( mRightAxis.mWorldTransform );
		mUpAxis.mRenderable.SetTransform( mUpAxis.mWorldTransform );
		mUpAxisArrow.mRenderable.SetTransform( mUpAxisArrow.mWorldTransform );
		mForwardAxisArrow.mRenderable.SetTransform( mForwardAxisArrow.mWorldTransform );
		mRightAxisArrow.mRenderable.SetTransform( mRightAxisArrow.mWorldTransform );
		mYZAxis.mRenderable.SetTransform( mYZAxis.mWorldTransform );
		mXYAxis.mRenderable.SetTransform( mXYAxis.mWorldTransform );
		mXZAxis.mRenderable.SetTransform( mXZAxis.mWorldTransform );
	}

	Transform EditorTranslationWidget::GetWorldTransform( )
	{
		Update( );
		return mRoot.mWorldTransform;
	}

	// Sets root position
	void EditorTranslationWidget::SetPosition( const Vec3& position )
	{
		// Set position and update everything else
		mRoot.mLocalTransform.SetPosition( position );
		Update( );
	}

	// Sets root scale
	void EditorTranslationWidget::SetScale( const f32& scale )
	{
		mRoot.mLocalTransform.SetScale( scale );
		Update( );
	}

	void EditorTranslationWidget::SetRotation( const Quaternion& rotation )
	{
		mRoot.mLocalTransform.SetRotation( rotation );
		Update( );
	}
}
