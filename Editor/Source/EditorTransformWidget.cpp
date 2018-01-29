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
		mTranslationWidget.Initialize( this );

		// Initialize the scale widget
		mScaleWidget.Initialize( this );

		// Set transformation mode to translation
		SetTransformationMode( TransformationMode::Translation );
	}

	void EditorTransformWidget::SetTransformationMode( TransformationMode mode )
	{
		// Do nothing if already in this mode
		if ( mMode == mode && mActiveWidget != nullptr ) 
		{
			return;
		}

		EditorWidget* previousWidget = nullptr;

		// Disable previous widget for mode
		if ( mActiveWidget )
		{
			previousWidget = mActiveWidget;
			mActiveWidget->Disable( ); 
		}

		// Set mode
		mMode = mode; 

		// Set newly active widget
		switch ( mMode )
		{
			case TransformationMode::Translation:
			{
				mActiveWidget = &mTranslationWidget;
			} break;

			case TransformationMode::Scale:
			{
				mActiveWidget = &mScaleWidget;
			} break;
		}

		// If previous widget available, set transform of that widget to this one
		if ( previousWidget )
		{
			mActiveWidget->SetTransform( previousWidget->GetWorldTransform( ) ); 
		}

		// Enable widget
		mActiveWidget->Enable( );
	}

	void EditorTransformWidget::Update( )
	{ 
		// Update active widget
		mActiveWidget->Update( ); 
	}

	Transform EditorTransformWidget::GetWorldTransform( )
	{ 
		return mActiveWidget->GetWorldTransform( );
		//return mTranslationWidget.GetWorldTransform( );
	}

	void EditorTransformWidget::SetPosition( const Vec3& position )
	{
		mActiveWidget->SetPosition( position );
		//mTranslationWidget.SetPosition( position );
	}

	void EditorTransformWidget::SetScale( const f32& scale )
	{
		mActiveWidget->SetScale( scale );
		//mTranslationWidget.SetScale( scale );
	}

	void EditorTransformWidget::SetRotation( const Quaternion& rotation )
	{
		mActiveWidget->SetRotation( rotation );
	}

	TransformationMode EditorTransformWidget::GetTransformationMode()
	{
		return mMode;
	}

	LineIntersectionResult EditorTransformWidget::GetLineIntersectionResult( const Vec3& axisA, const Vec3& axisB, const Vec3& axisC, bool compareSupportingAxes, const Vec3& axisToUseAsPlaneNormal ) 
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetSceneCamera( )->ConstCast< Camera >( );

		// Find dot between cam forward and right axis
		Vec3 cF = camera->Forward( ).Normalize( );
		Vec3 Ta = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis( ) * axisA ).Normalize( );
		f32 cFDotTa = std::fabs( cF.Dot( Ta ) ); 

		Plane intersectionPlane;
		Vec3 oP = mTranslationWidget.mRoot.mWorldTransform.GetPosition( );

		if ( compareSupportingAxes )
		{
			// Now determine appropriate axis to move along
			Vec3 Tb = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis( ) * axisB ).Normalize( );
			Vec3 Tc = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis( ) * axisC ).Normalize( );

			f32 cFDotTb = std::fabs( cF.Dot( Tb ) );
			f32 cFDotTc = std::fabs( cF.Dot( Tc ) ); 

			// Define intersection plane using most orthogonal axis
			intersectionPlane = cFDotTb < cFDotTc ? Plane( Tc, oP ) : Plane( Tb, oP ); 
		}
		else
		{
			// Plane defined by axis normal and transform point
			 intersectionPlane = Plane( axisToUseAsPlaneNormal, oP ); 
		}

		// Get ray from camera
		Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );

		// Get intersection result of plane 
		LineIntersectionResult intersectionResult = intersectionPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection );

		return intersectionResult; 
	}

	LineIntersectionResult EditorTransformWidget::GetLineIntersectionResultSingleAxis( const Vec3& axis )
	{
		return GetLineIntersectionResult( axis, Vec3( ), Vec3( ), false, axis );
	}

	void EditorTransformWidget::InteractWithWidget( )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetSceneCamera( )->ConstCast< Camera >( );

		if ( mInteractingWithTransformWidget )
		{
			switch ( mType )
			{
				case ( TransformWidgetRenderableType::TranslationRoot ):
				{
					// Get intersection result
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( camera->Forward( ).Normalize( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						// Calculate delta
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Store delta
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationRightAxis ):
				{
					// Right axis transformed by orientation
					Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::XAxis( ) ).Normalize( ); 

					// Get intersection result
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::XAxis(), Vec3::YAxis(), Vec3::ZAxis() ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Calculate delta from starting position ( lock to x axis )
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Tx );

						// Store delta as final projection
						mDelta = Tx * uDotN;

						// Reset start position
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						// Clamp to x axis check
						if ( Tx == Vec3::XAxis( ) )
						{
							mDelta.y = 0.0f; 
							mDelta.z = 0.0f;
						}
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationForwardAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::ZAxis( ) ).Normalize( );

					// Get line intersection result
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::ZAxis( ), Vec3::XAxis( ), Vec3::YAxis( ) );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Calculate delta from starting position ( lock to x axis )
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Tz );

						// Store delta as final projection
						mDelta = Tz * uDotN; 

						// Reset start position 
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						// Clamp to z axis check
						if ( Tz == Vec3::ZAxis( ) )
						{
							mDelta.x = 0.0f; 
							mDelta.y = 0.0f;
						}
					}
				} break;

				case ( TransformWidgetRenderableType::TranslationUpAxis ) :
				{
					Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::YAxis( ) ).Normalize( );
 
					// Get line intersection result
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::YAxis( ), Vec3::XAxis( ), Vec3::ZAxis( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						// Calculate delta from starting position ( lock to x axis )
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Ty );

						// Store delta as final projection
						mDelta = Ty * uDotN;

						// Reset start position
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						// Clamp to y axis check 
						if ( Ty == Vec3::YAxis( ) )
						{
							mDelta.x = 0.0f; 
							mDelta.z = 0.0f;
						}
					}
				} break;

				case ( TransformWidgetRenderableType::TranslationXYAxes ):
				{ 
					// Axis of rotation
					Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::ZAxis( ) ).Normalize( );

					// Get line intersection result with plane
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tz );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Clamp z axis check
						if ( Tz == Vec3::ZAxis( ) )
						{
							mDelta.z = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationXZAxes ):
				{ 
					// Axis of rotation
					Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3( 0.0f, 1.0f, 0.0f ) ).Normalize( ); 

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Ty );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Clamp to y axis check
						if ( Ty == Vec3::YAxis( ) )
						{
							mDelta.y = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationYZAxes ):
				{ 
					// Axis of rotation
					Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::XAxis( ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tx );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Clamp to x axis check
						if ( Tx == Vec3::XAxis( ) )
						{
							mDelta.x = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleRightAxis ):
				{
					// Right axis transformed by orientation
					Vec3 Tx = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::XAxis( ) ).Normalize( ); 

					f32 TxDotXAxis = Tx.Dot( Vec3::XAxis( ) );

					Vec3 axis = TxDotXAxis < 0.03f ? -Vec3::XAxis( ) : Vec3::XAxis( ); 
					f32 negation = TxDotXAxis < 0.03f ? -1.0f : 1.0f;

					// Get line intersection result 
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( axis, Vec3::XAxis( ), Vec3::ZAxis( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Calculate delta from starting position ( lock to x axis ) 
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Tx );

						// Store delta as final projection
						mDelta = Tx * uDotN; 

						// Reset start position
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						mDelta *= -negation;

						// Clamp to x axis check
						mDelta.y = 0.0f; 
						mDelta.z = 0.0f;
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleForwardAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 Tz = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::ZAxis( ) ).Normalize( );
 
					f32 TzDotZAxis = Tz.Dot( Vec3::ZAxis( ) ); 

					Vec3 axis = TzDotZAxis < 0.03f ? -Vec3::ZAxis( ) : Vec3::ZAxis( ); 
					f32 negation = TzDotZAxis < 0.03f ? -1.0f : 1.0f;

					// Get line intersection result 
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( axis, Vec3::XAxis( ), Vec3::ZAxis( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Calculate delta from starting position ( lock to x axis )
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Tz );

						// Store delta as final projection
						mDelta = Tz * uDotN; 

						// Reset start position 
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						mDelta *= negation;

						// Clamp to z axis check
						mDelta.x = 0.0f; 
						mDelta.y = 0.0f;
					}
				} break;

				case ( TransformWidgetRenderableType::ScaleUpAxis ) :
				{
					Vec3 Ty = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::YAxis( ) ).Normalize( );
 
					f32 TyDotYAxis = Ty.Dot( Vec3::YAxis( ) );

					Vec3 axis = TyDotYAxis < 0.03f ? -Vec3::YAxis( ) : Vec3::YAxis( ); 
					f32 negation = TyDotYAxis < 0.03f ? -1.0f : 1.0f;

					// Get line intersection result 
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( axis, Vec3::XAxis( ), Vec3::ZAxis( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						// Calculate delta from starting position ( lock to x axis )
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Ty );

						// Store delta as final projection
						mDelta = Ty * uDotN;

						// Reset start position
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						mDelta *= negation;

						// Clamp to y axis check 
						mDelta.x = 0.0f; 
						mDelta.z = 0.0f;
					}
				} break;

				case ( TransformWidgetRenderableType::ScaleYZAxes ):
				{ 
					// Axis of rotation
					Vec3 Tx = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::XAxis( ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tx );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Clamp to x axis check
						if ( Tx == Vec3::XAxis( ) )
						{
							mDelta.x = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleXYAxes ):
				{ 
					// Axis of rotation
					Vec3 Tz = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::ZAxis( ) ).Normalize( );

					// Get line intersection result with plane
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tz );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Clamp z axis check
						mDelta.z = 0.0f; 

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleXZAxes ):
				{ 
					// Axis of rotation
					Vec3 Ty = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3( 0.0f, 1.0f, 0.0f ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Ty );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Clamp to y axis check
						mDelta.y = 0.0f; 

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

		if ( mActiveWidget )
		{
			mActiveWidget->EndInteraction( mType );
		}
	}

	void EditorTransformWidget::StoreIntersectionResultInformation( const LineIntersectionResult& result, TransformWidgetRenderableType type )
	{
		if ( result.mHit )
		{
			const Vec3* hp = &result.mHitPosition; 
			// Store position and then store widget that's being operated upon
			mInteractingWithTransformWidget = true;
			mType = type;
			mIntersectionStartPosition = result.mHitPosition;
			mRootStartPosition = mTranslationWidget.mRoot.mWorldTransform.GetPosition( );
		} 
	} 

	void EditorTransformWidget::BeginWidgetInteraction( TransformWidgetRenderableType type )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetSceneCamera( )->ConstCast< Camera >();

		// Begin interaction with active widget
		if ( mActiveWidget )
		{
			mActiveWidget->BeginInteraction( type );
		}

		// Look for picked transform widget
		switch ( type )
		{ 
			case ( TransformWidgetRenderableType::TranslationRoot ):
			{
				// Axis of rotation
				Vec3 cF = camera->Forward( ).Normalize( ); 

				// Do intersection test
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( cF );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationRoot ); 
			} break;

			

			case ( TransformWidgetRenderableType::TranslationRightAxis ):
			{ 
				// Get line intersection result
				LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::XAxis( ), Vec3::YAxis( ), Vec3::ZAxis( ) ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationRightAxis ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationUpAxis ):
			{ 
				// Get line intersection result
				LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::YAxis( ), Vec3::XAxis( ), Vec3::ZAxis( ) ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationUpAxis ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationForwardAxis ):
			{ 
				// Get line intersection result
				LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::ZAxis( ), Vec3::YAxis( ), Vec3::XAxis( ) ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationForwardAxis ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationXYAxes ):
			{ 
				// Axis of rotation
				Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation().NegativeAngleAxis() * Vec3::ZAxis() ).Normalize(); 

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tz );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationXYAxes ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationXZAxes ):
			{ 
				// Axis of rotation
				Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::YAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Ty );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationXZAxes ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationYZAxes ):
			{ 
				// Axis of rotation
				Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::XAxis( ) ).Normalize( ); 

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tx );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationYZAxes ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleRightAxis ):
			{ 
				// Get line intersection result
				LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::XAxis( ), Vec3::YAxis( ), Vec3::ZAxis( ) ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleRightAxis ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleForwardAxis ):
			{ 
				// Get line intersection result
				LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::ZAxis( ), Vec3::YAxis( ), Vec3::XAxis( ) ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleForwardAxis ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleUpAxis ):
			{ 
				// Get line intersection result
				LineIntersectionResult intersectionResult = GetLineIntersectionResult( Vec3::YAxis( ), Vec3::ZAxis( ), Vec3::XAxis( ) ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleUpAxis ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleYZAxes ):
			{ 
				// Axis of rotation
				Vec3 Tx = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::XAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tx );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleYZAxes ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleXYAxes ):
			{ 
				// Axis of rotation
				Vec3 Tz = ( mScaleWidget.mRoot.mWorldTransform.GetRotation().NegativeAngleAxis() * Vec3::ZAxis() ).Normalize();

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Tz );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleXYAxes ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleXZAxes ):
			{ 
				// Axis of rotation
				Vec3 Ty = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ).NegativeAngleAxis() * Vec3::YAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxis( Ty );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleXZAxes ); 
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
