// @file EditorTranslationWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include <Engine.h>
#include <Asset/AssetManager.h>
#include <Graphics/GraphicsSubsystem.h>
#include <SubsystemCatalog.h>

#include "EditorTranslationWidget.h"

namespace Enjon
{
	void EditorTranslationWidget::Initialize( )
	{ 
		// Set up renderables for each of the axes
		AssetManager* am = EngineSubsystem( AssetManager );

		// Set up renderables
		mForwardAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cylinder" ) );
		mRightAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cylinder" ) );
		mUpAxis.mRenderable.SetMesh( am->GetAsset< Mesh >( "models.unit_cylinder" ) );
		mForwardAxis.mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ) );
		mRightAxis.mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ) );
		mUpAxis.mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ) );

		// Add to root
		mRoot.AddChild( &mForwardAxis );
		mRoot.AddChild( &mUpAxis );
		mRoot.AddChild( &mRightAxis );

		// Set up local transforms for each
		f32 yScale = 1.0f;
		f32 xzScale = 0.05f;
		f32 bias = 2.5f;
		mForwardAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( 90.0f ), Vec3::XAxis() ) );
		mRightAxis.mLocalTransform.SetRotation( Quaternion::AngleAxis( ToRadians( 90.0f ), Vec3::ZAxis() ) );
		mForwardAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mRightAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mUpAxis.mLocalTransform.SetScale( Vec3( xzScale, yScale, xzScale ) );
		mForwardAxis.mLocalTransform.SetPosition( Vec3( 0.0f, 0.0f, yScale ) );
		mRightAxis.mLocalTransform.SetPosition( Vec3( yScale, 0.0f, 0.0f ) );
		mUpAxis.mLocalTransform.SetPosition( Vec3( 0.0f, yScale, 0.0f ) );

		// Add renderables to graphics scene
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		gfx->GetScene( )->AddRenderable( &mForwardAxis.mRenderable );
		gfx->GetScene( )->AddRenderable( &mRightAxis.mRenderable );
		gfx->GetScene( )->AddRenderable( &mUpAxis.mRenderable ); 
	} 

	void EditorTranslationWidget::Update( )
	{
		// Calculate world transforms for all
		mRoot.CalculateWorldTransform( ); 
		mForwardAxis.CalculateWorldTransform( );
		mRightAxis.CalculateWorldTransform( );
		mUpAxis.CalculateWorldTransform( );

		// Set up transforms for renderables
		mForwardAxis.mRenderable.SetTransform( mForwardAxis.mWorldTransform );
		mRightAxis.mRenderable.SetTransform( mRightAxis.mWorldTransform );
		mUpAxis.mRenderable.SetTransform( mUpAxis.mWorldTransform );
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
}
