// @file EditorTransformWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "EditorTransformWidget.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Engine.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>

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

	void EditorTransformWidget::SetRotation( const Quaternion& rotation )
	{
		mTranslationWidget.SetRotation( rotation );
	}

	void EditorTransformWidget::InteractWithWidget( )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetSceneCamera( )->ConstCast< Camera >( );

		// Activate translation widget
		mTranslationWidget.Activate( mType );

		if ( mInteractingWithTransformWidget )
		{
			switch ( mType )
			{
				case ( TransformWidgetRenderableType::TranslationRoot ):
				{
					// Find dot between cam forward and right axis
					Vec3 cF = camera->Forward( ).Normalize( ); 
					Vec3 oP = mTranslationWidget.mRoot.mWorldTransform.GetPosition( );

					// Define object plane
					Plane objectPlane = Plane( cF, oP );

					// Define ray from mouse position on screen to world plane
					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Do intserection test
					LineIntersectionResult intersectionResult = objectPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationRightAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 cF = camera->Forward( ).Normalize( );
					Vec3 Tx = Vec3( 1.0f, 0.0f, 0.0f );
					f32 cFDotTx = std::fabs( cF.Dot( Tx ) );

					// Can't continue with movement if directly parallel to axis
					if ( cFDotTx > 1.0f )
					{
						break;
					}

					// Now determine appropriate axis to move along
					Vec3 Ty = Vec3( 0.0f, 1.0f, 0.0f );
					Vec3 Tz = Vec3( 0.0f, 0.0f, 1.0f );

					f32 cFDotTy = std::fabs( cF.Dot( Ty ) );
					f32 cFDotTz = std::fabs( cF.Dot( Tz ) );

					LineIntersectionResult intersectionResult;

					// Choose to use XY-plane
					if ( cFDotTy < cFDotTz )
					{
						// Need to define z as normal to plane
						Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) );

						// Define XZ plane
						Plane XYPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

						// Define ray from mouse position on screen to world plane
						Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

						// Get intersection result of plane 
						intersectionResult = XYPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 
					}
					// Choose to use XZ-plane
					else
					{ 
						// Need to define z as normal to plane
						Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) );

						// Define XZ plane
						Plane XZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

						// Define ray from mouse position on screen to world plane
						Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

						// Get intersection result of plane 
						intersectionResult = XZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 
					}

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to x axis )
						mDelta = Vec3( ( intersectionResult.mHitPosition - mIntersectionStartPosition ).x, 0.0f, 0.0f ); 

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition; 
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationForwardAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 cF = camera->Forward( ).Normalize( );
					Vec3 Tz = Vec3( 0.0f, 0.0f, 1.0f );
					f32 cFDotTz = std::fabs( cF.Dot( Tz ) );

					// Can't continue with movement if directly parallel to axis
					if ( cFDotTz > 1.0f )
					{
						break;
					}

					// Now determine appropriate axis to move along
					Vec3 Ty = Vec3( 0.0f, 1.0f, 0.0f );
					Vec3 Tx = Vec3( 1.0f, 0.0f, 0.0f );

					f32 cFDotTy = std::fabs( cF.Dot( Ty ) );
					f32 cFDotTx = std::fabs( cF.Dot( Tx ) );

					LineIntersectionResult intersectionResult;

					// Choose to use YZ-plane
					if ( cFDotTy < cFDotTx )
					{
						// Need to define z as normal to plane
						Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) );

						// Define YZ plane
						Plane YZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

						// Define ray from mouse position on screen to world plane
						Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

						// Get intersection result of plane 
						intersectionResult = YZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
					}
					// Choose to use XZ-plane
					else
					{
						// Need to define z as normal to plane
						Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) );

						// Define XZ plane
						Plane XZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

						// Define ray from mouse position on screen to world plane
						Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

						// Get intersection result of plane 
						intersectionResult = XZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
					}

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Calculate delta from starting position ( lock to x axis )
						mDelta = Vec3( 0.0f, 0.0f, ( intersectionResult.mHitPosition - mIntersectionStartPosition ).z ); 

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition; 
					}
				} break;

				case ( TransformWidgetRenderableType::TranslationUpAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 cF = camera->Forward( ).Normalize( );
					Vec3 Ty = Vec3( 0.0f, 1.0f, 0.0f );
					f32 cFDotTy = std::fabs( cF.Dot( Ty ) );

					// Can't continue with movement if directly parallel to axis
					if ( cFDotTy > 1.0f )
					{
						break;
					}

					// Now determine appropriate axis to move along
					Vec3 Tz = Vec3( 0.0f, 0.0f, 1.0f );
					Vec3 Tx = Vec3( 1.0f, 0.0f, 0.0f );

					f32 cFDotTz = std::fabs( cF.Dot( Tz ) );
					f32 cFDotTx = std::fabs( cF.Dot( Tx ) );

					LineIntersectionResult intersectionResult;

					// Choose to use YZ-plane
					if ( cFDotTz < cFDotTx )
					{
						// Need to define z as normal to plane
						Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) );

						// Define YZ plane
						Plane YZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

						Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

						// Get intersection result of plane 
						intersectionResult = YZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
					}
					// Choose to use XY-plane
					else
					{ 
						// Need to define z as normal to plane
						Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) );

						// Define XY plane
						Plane XYPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

						Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

						// Get intersection result of plane 
						intersectionResult = XYPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
					}

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;
 
						// Calculate delta from starting position ( lock to x axis )
						mDelta = Vec3( 0.0f, ( intersectionResult.mHitPosition - mIntersectionStartPosition ).y, 0.0f ); 

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition; 
					}
				} break;
			} 
		}
	}

	void EditorTransformWidget::EndInteraction( )
	{
		mInteractingWithTransformWidget = false;
		mTranslationWidget.Deactivate( mType );
	}

	void EditorTransformWidget::BeginWidgetInteraction( TransformWidgetRenderableType type )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetSceneCamera( )->ConstCast< Camera >();

		// Look for picked transform widget
		switch ( type )
		{ 
			case ( TransformWidgetRenderableType::TranslationRoot ):
			{
				// Find dot between cam forward and right axis
				Vec3 cF = camera->Forward( ).Normalize( ); 
				Vec3 oP = mTranslationWidget.mRoot.mWorldTransform.GetPosition( ); 

				// Define object plane
				Plane objectPlane = Plane( cF, oP );

				// Define ray from mouse position on screen to world plane
				Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

				// Do intersection test
				LineIntersectionResult intersectionResult = objectPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 

				if ( intersectionResult.mHit )
				{
					const Vec3* hp = &intersectionResult.mHitPosition;

					// Store position and then store that we're moving
					mInteractingWithTransformWidget = true;
					mType = TransformWidgetRenderableType::TranslationRoot;
					mIntersectionStartPosition = intersectionResult.mHitPosition;
					mRootStartPosition = mTranslationWidget.mRoot.mWorldTransform.GetPosition( );
				}

			} break;
			case ( TransformWidgetRenderableType::TranslationRightAxis ):
			{ 
				// Find dot between cam forward and right axis
				Vec3 cF = camera->Forward( ).Normalize( );
				Vec3 Tx = Vec3( 1.0f, 0.0f, 0.0f );
				f32 cFDotTx = std::fabs( cF.Dot( Tx ) );

				// Can't continue with movement if directly parallel to axis
				if ( cFDotTx > 1.0f )
				{
					break;
				} 

				// Now determine appropriate axis to move along
				Vec3 Ty = Vec3( 0.0f, 1.0f, 0.0f );
				Vec3 Tz = Vec3( 0.0f, 0.0f, 1.0f );

				f32 cFDotTy = std::fabs( cF.Dot( Ty ) );
				f32 cFDotTz = std::fabs( cF.Dot( Tz ) ); 

				LineIntersectionResult intersectionResult;

				// Choose to use XY-plane
				if ( cFDotTy < cFDotTz )
				{
					// Need to define z as normal to plane
					Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation() * Vec3::ZAxis( ) );

					// Define XZ plane
					Plane XYPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

					// Define ray from mouse position on screen to world plane
					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Get intersection result of plane 
					intersectionResult = XYPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 
				} 
				// Choose to use XZ-plane
				else
				{
					// Need to define z as normal to plane
					Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) );

					// Define XZ plane
					Plane XZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

					// Define ray from mouse position on screen to world plane
					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Get intersection result of plane 
					intersectionResult = XZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 
				} 

				if ( intersectionResult.mHit )
				{
					const Vec3* hp = &intersectionResult.mHitPosition; 

					// Store position and then store that we're moving
					mInteractingWithTransformWidget = true;
					mType = TransformWidgetRenderableType::TranslationRightAxis;
					mIntersectionStartPosition = intersectionResult.mHitPosition;
					mRootStartPosition = mTranslationWidget.mRoot.mWorldTransform.GetPosition( ); 
				}

			} break;

			case ( TransformWidgetRenderableType::TranslationUpAxis ):
			{ 
				// Find dot between cam forward and right axis
				Vec3 cF = camera->Forward( ).Normalize( );
				Vec3 Ty = Vec3( 0.0f, 1.0f, 0.0f );
				f32 cFDotTy = std::fabs( cF.Dot( Ty ) );

				// Can't continue with movement if directly parallel to axis
				if ( cFDotTy > 1.0f )
				{
					break;
				}

				// Now determine appropriate axis to move along
				Vec3 Tx = Vec3( 1.0f, 0.0f, 0.0f );
				Vec3 Tz = Vec3( 0.0f, 0.0f, 1.0f );

				f32 cFDotTx = std::fabs( cF.Dot( Tx ) );
				f32 cFDotTz = std::fabs( cF.Dot( Tz ) );
 
				LineIntersectionResult intersectionResult;

				// Choose to use XY-plane
				if ( cFDotTx < cFDotTz )
				{
					// Need to define z as normal to plane
					Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) );

					// Define XZ plane
					Plane XYPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Get intersection result of plane 
					intersectionResult = XYPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
				}
				// Choose to use YZ-plane
				else
				{ 
					// Need to define z as normal to plane
					Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) );

					// Define XZ plane
					Plane YZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Get intersection result of plane 
					intersectionResult = YZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
				}

				if ( intersectionResult.mHit )
				{
					const Vec3* hp = &intersectionResult.mHitPosition;

					// Store position and then store that we're moving
					mInteractingWithTransformWidget = true;
					mType = TransformWidgetRenderableType::TranslationUpAxis;
					mIntersectionStartPosition = intersectionResult.mHitPosition;
					mRootStartPosition = mTranslationWidget.mRoot.mWorldTransform.GetPosition( );
				}

			} break;

			case ( TransformWidgetRenderableType::TranslationForwardAxis ):
			{ 
				// Find dot between cam forward and right axis
				Vec3 cF = camera->Forward( ).Normalize( );
				Vec3 Tz = Vec3( 0.0f, 0.0f, -1.0f );
				f32 cFDotTz = std::fabs( cF.Dot( Tz ) );

				// Can't continue with movement if directly parallel to axis
				if ( cFDotTz > 1.0f )
				{
					break;
				}

				// Now determine appropriate axis to move along
				Vec3 Tx = Vec3( 1.0f, 0.0f, 0.0f );
				Vec3 Ty = Vec3( 0.0f, 1.0f, 0.0f );

				f32 cFDotTx = std::fabs( cF.Dot( Tx ) );
				f32 cFDotTy = std::fabs( cF.Dot( Ty ) );
 
				LineIntersectionResult intersectionResult;

				// Choose to use XZ-plane
				if ( cFDotTx < cFDotTy )
				{
					// Need to define z as normal to plane
					Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) );

					// Define XZ plane
					Plane XZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Get intersection result of plane 
					intersectionResult = XZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
				}
				// Choose to use YZ-plane
				else
				{ 
					// Need to define z as normal to plane
					Vec3 normal = Vec3::Normalize( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) );

					// Define XZ plane
					Plane YZPlane( normal, mTranslationWidget.mRoot.mWorldTransform.GetPosition( ) );

					Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

					// Get intersection result of plane 
					intersectionResult = YZPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );
				}

				if ( intersectionResult.mHit )
				{
					const Vec3* hp = &intersectionResult.mHitPosition;
 
					// Store position and then store that we're moving
					mInteractingWithTransformWidget = true;
					mType = TransformWidgetRenderableType::TranslationForwardAxis;
					mIntersectionStartPosition = intersectionResult.mHitPosition;
					mRootStartPosition = mTranslationWidget.mRoot.mWorldTransform.GetPosition( );
				}

			} break;
		}
	}

	bool EditorTransformWidget::IsInteractingWithWidget( ) const
	{
		return mInteractingWithTransformWidget;
	}

	Vec3 EditorTransformWidget::GetDelta( ) const
	{
		return mDelta;
	}

	TransformWidgetRenderableType EditorTransformWidget::GetInteractedWidgetType( )
	{
		return mType;
	}
}
