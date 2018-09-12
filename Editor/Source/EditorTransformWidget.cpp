// @file EditorTransformWidget.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "EditorTransformWidget.h" 
#include "EditorApp.h"

#include <Graphics/GraphicsSubsystem.h>
#include <Engine.h>
#include <SubsystemCatalog.h>
#include <IO/InputManager.h>

#include <fmt/format.h>

namespace Enjon
{
	void EditorTransformWidget::Initialize( EditorApp* app )
	{
		// Initialize the translation widget
		mTranslationWidget.Initialize( this );

		// Initialize the scale widget
		mScaleWidget.Initialize( this );

		// Initialize the rotation widget
		mRotationWidget.Initialize( this );

		// Set transformation mode to translation
		SetTransformationMode( TransformationMode::Translation );

		// Deactivate current widget
		mActiveWidget->Disable( );

		// Set editor app
		mEditorApp = app; 

		for ( u32 i = 0; i < ( u32 )TransformationMode::Count; ++i )
		{
			mSnapEnabled[ i ] = true;
		}
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

		// Set newly active widget
		switch ( mode )
		{
			case TransformationMode::Translation:
			{
				mActiveWidget = &mTranslationWidget;
			} break;

			case TransformationMode::Scale:
			{ 
				mActiveWidget = &mScaleWidget;

				// Set transform space to local
				SetTransformSpace( TransformSpace::Local );

			} break;

			case TransformationMode::Rotation:
			{
				mActiveWidget = &mRotationWidget;
			}
		}

		// Set mode
		mMode = mode; 

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

	void EditorTransformWidget::Enable( bool enable )
	{
		if ( enable == mEnabled )
		{
			return;
		}

		// Set enabled state
		mEnabled = enable;

		if ( mEnabled )
		{
			mActiveWidget->Enable( );
		}
		else
		{
			mActiveWidget->Disable( );
		}
	}

	Transform EditorTransformWidget::GetWorldTransform( )
	{ 
		return mActiveWidget->GetWorldTransform( );
	}

	void EditorTransformWidget::SetPosition( const Vec3& position )
	{
		mActiveWidget->SetPosition( position );
	}

	void EditorTransformWidget::SetScale( const f32& scale )
	{
		mActiveWidget->SetScale( scale );
	}

	void EditorTransformWidget::SetRotation( const Quaternion& rotation )
	{
		if ( mTransformSpace == TransformSpace::Local )
		{
			mActiveWidget->SetRotation( rotation ); 
		}
	}

	void EditorTransformWidget::SetRotation( const Vec3& eulerAngles )
	{
		if ( mTransformSpace == TransformSpace::Local )
		{
			mActiveWidget->SetRotation( eulerAngles );
		}
	}

	TransformationMode EditorTransformWidget::GetTransformationMode()
	{
		return mMode;
	}

	TransformSpace EditorTransformWidget::GetTransformSpace( ) const
	{
		return mTransformSpace;
	} 

	void EditorTransformWidget::SetTransformSpace( TransformSpace space )
	{
		if ( mMode != TransformationMode::Scale )
		{
			mTransformSpace = space;

			// Reset rotation if world space
			if ( space == TransformSpace::World )
			{
				mActiveWidget->SetRotation( Quaternion( ) );
			} 
		}
	}

	LineIntersectionResult EditorTransformWidget::GetLineIntersectionResult( const Vec3& axisA, const Vec3& axisB, const Vec3& axisC, bool compareSupportingAxes, bool overrideAxis, const Vec3& axisToUseAsPlaneNormal ) 
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetGraphicsSceneCamera( )->ConstCast< Camera >( );

		// Find dot between cam forward and right axis
		Vec3 cF = camera->Forward( ).Normalize( );
		Vec3 Ta = ( mActiveWidget->GetWorldTransform().GetRotation( ) * axisA ).Normalize( );
		f32 cFDotTa = std::fabs( cF.Dot( Ta ) ); 

		Plane intersectionPlane;
		Vec3 oP = mActiveWidget->GetWorldTransform().GetPosition( );

		if ( compareSupportingAxes )
		{
			// Now determine appropriate axis to move along
			Vec3 Tb = ( mActiveWidget->GetWorldTransform().GetRotation( ) * axisB ).Normalize( );
			Vec3 Tc = ( mActiveWidget->GetWorldTransform().GetRotation( ) * axisC ).Normalize( );

			f32 cFDotTb = std::fabs( cF.Dot( Tb ) );
			f32 cFDotTc = std::fabs( cF.Dot( Tc ) ); 

			// Define intersection plane using most orthogonal axis
			intersectionPlane = cFDotTb < cFDotTc ? Plane( Tc, oP ) : Plane( Tb, oP ); 
		}
		else
		{
			if ( overrideAxis )
			{
				// Plane defined by axis normal and transform point
				 intersectionPlane = Plane( axisToUseAsPlaneNormal, oP ); 
			}
			else
			{
				// Plane defined by axis normal and transform point
				 intersectionPlane = Plane( Ta, oP ); 
			}
		}

		// Get ray from camera
		//Ray ray = camera->ScreenToWorldRay( input->GetMouseCoords( ) );
		Ray ray = camera->ScreenToWorldRay( mEditorApp->GetSceneViewProjectedCursorPosition( ) );

		// Get intersection result of plane 
		LineIntersectionResult intersectionResult = intersectionPlane.GetLineIntersection( ray.mPoint, ray.mPoint + ray.mDirection ); 

		return intersectionResult; 
	} 

	LineIntersectionResult EditorTransformWidget::GetLineIntersectionResultSingleAxis( const Vec3& axis )
	{
		return GetLineIntersectionResult( axis, Vec3( ), Vec3( ), false, false, axis );
	}

	LineIntersectionResult EditorTransformWidget::GetLineIntersectionResultSingleAxisOverride( const Vec3& axis )
	{
		return GetLineIntersectionResult( axis, Vec3( ), Vec3( ), false, true, axis );
	}

	void EditorTransformWidget::InteractWithWidget( Transform* transform )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetGraphicsSceneCamera( )->ConstCast< Camera >( );

		if ( mInteractingWithTransformWidget )
		{
			switch ( mType )
			{
				case ( TransformWidgetRenderableType::TranslationRoot ):
				{
					// Get intersection result
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( camera->Forward( ).Normalize( ) ); 

					// Right axis transformed by orientation
					Vec3 Tx = ( camera->Right( ) ).Normalize( );
					Vec3 Ty = ( camera->Up( ) ).Normalize( );
					Vec3 Tz = ( camera->Forward( ) ).Normalize( );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						// Calculate delta
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Store delta
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition; 

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							Vec3 u = mAccumulatedTranslationDelta;
	 
							// Need to project u onto n
							f32 uXDotN = u.Dot( Tx );

							// Store delta as final projection
							Vec3 xDelta = Tx * uXDotN; 
	 
							// Need to project u onto n
							f32 uYDotN = u.Dot( Ty );

							// Store delta as final projection
							Vec3 yDelta = Ty * uYDotN; 

							// Need to project u onto n
							f32 uZDotN = u.Dot( Tz );

							// Store delta as final projection
							Vec3 zDelta = Tz * uZDotN; 

							f32 xSign = uXDotN >= 0.0f ? 1.0f : -1.0f;
							f32 ySign = uYDotN >= 0.0f ? 1.0f : -1.0f;
							f32 zSign = uZDotN >= 0.0f ? 1.0f : -1.0f;
 
							xDelta = Tx * Math::Round( xDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * xSign;
							yDelta = Ty * Math::Round( yDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * ySign;
							zDelta = Tz * Math::Round( zDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * zSign;

							finalDelta = xDelta + yDelta + zDelta; 
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationRightAxis ):
				{
					// Right axis transformed by orientation
					Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( ); 

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

						// Math::Clamp to x axis check
						if ( Tx == Vec3::XAxis( ) )
						{
							mDelta.y = 0.0f; 
							mDelta.z = 0.0f;
						}

						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							f32 snapLen = Math::Round( mAccumulatedTranslationDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( );
							finalDelta = mAccumulatedTranslationDelta.Normalize( ) * snapLen;
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationForwardAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );

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

						// Math::Clamp to z axis check
						if ( Tz == Vec3::ZAxis( ) )
						{
							mDelta.x = 0.0f; 
							mDelta.y = 0.0f;
						}
						
						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							f32 snapLen = Math::Round( mAccumulatedTranslationDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( );
							finalDelta = mAccumulatedTranslationDelta.Normalize( ) * snapLen;
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}
				} break;

				case ( TransformWidgetRenderableType::TranslationUpAxis ) :
				{
					Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) ).Normalize( );
 
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

						// Math::Clamp to y axis check 
						if ( Ty == Vec3::YAxis( ) )
						{
							mDelta.x = 0.0f; 
							mDelta.z = 0.0f;
						}

						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							f32 snapLen = Math::Round( mAccumulatedTranslationDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( );
							finalDelta = mAccumulatedTranslationDelta.Normalize( ) * snapLen;
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}
				} break;

				case ( TransformWidgetRenderableType::TranslationXYAxes ):
				{ 
					// Axis of rotation
					Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );

					Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( );
					Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) ).Normalize( );

					// Get line intersection result with plane
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tz );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Math::Clamp z axis check
						if ( Tz == Vec3::ZAxis( ) )
						{
							mDelta.z = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;

						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							Vec3 u = mAccumulatedTranslationDelta;
	 
							// Need to project u onto n
							f32 uXDotN = u.Dot( Tx );

							// Store delta as final projection
							Vec3 xDelta = Tx * uXDotN; 
	 
							// Need to project u onto n
							f32 uYDotN = u.Dot( Ty );

							// Store delta as final projection
							Vec3 yDelta = Ty * uYDotN; 

							f32 xSign = uXDotN >= 0.0f ? 1.0f : -1.0f;
							f32 ySign = uYDotN >= 0.0f ? 1.0f : -1.0f;
 
							xDelta = Tx * Math::Round( xDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * xSign;
							yDelta = Ty * Math::Round( yDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * ySign;

							finalDelta = xDelta + yDelta; 
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationXZAxes ):
				{ 
					// Axis of rotation
					Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) ).Normalize( );

					Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( );
					Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Ty );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Math::Clamp to y axis check
						if ( Ty == Vec3::YAxis( ) )
						{
							mDelta.y = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;

						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							Vec3 u = mAccumulatedTranslationDelta;
	 
							// Need to project u onto n
							f32 uXDotN = u.Dot( Tx );

							// Store delta as final projection
							Vec3 xDelta = Tx * uXDotN; 
	 
							// Need to project u onto n
							f32 uZDotN = u.Dot( Tz );

							// Store delta as final projection
							Vec3 zDelta = Tz * uZDotN; 

							f32 xSign = uXDotN >= 0.0f ? 1.0f : -1.0f;
							f32 zSign = uZDotN >= 0.0f ? 1.0f : -1.0f;
 
							xDelta = Tx * Math::Round( xDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * xSign;
							zDelta = Tz * Math::Round( zDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * zSign;

							finalDelta = xDelta + zDelta; 
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}

				} break;

				case ( TransformWidgetRenderableType::TranslationYZAxes ):
				{ 
					// Axis of rotation
					Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( );
					Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) ).Normalize( );
					Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tx );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Math::Clamp to x axis check
						if ( Tx == Vec3::XAxis( ) )
						{
							mDelta.x = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;

						// Store accumulated delta
						mAccumulatedTranslationDelta += mDelta;

						// Calculate final delta, based on snap settings
						Vec3 finalDelta = mAccumulatedTranslationDelta; 
						if ( mSnapEnabled[ ( u32 )TransformationMode::Translation ] )
						{
							Vec3 u = mAccumulatedTranslationDelta;
	 
							// Need to project u onto n
							f32 uYDotN = u.Dot( Ty );

							// Store delta as final projection
							Vec3 yDelta = Ty * uYDotN; 
	 
							// Need to project u onto n
							f32 uZDotN = u.Dot( Tz );

							// Store delta as final projection
							Vec3 zDelta = Tz * uZDotN; 

							f32 ySign = uYDotN >= 0.0f ? 1.0f : -1.0f;
							f32 zSign = uZDotN >= 0.0f ? 1.0f : -1.0f;
 
							yDelta = Ty * Math::Round( yDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * ySign;
							zDelta = Tz * Math::Round( zDelta.Length( ) / GetTranslationSnap( ) ) * GetTranslationSnap( ) * zSign;

							finalDelta = yDelta + zDelta; 
						}

						// Set final position
						transform->SetPosition( mRootTransform.GetPosition( ) + finalDelta );
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleRightAxis ):
				{
					Vec3 Tx = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( );
 
					f32 TxDotXAxis = Tx.Dot( Vec3::XAxis( ) );

					Vec3 axis = TxDotXAxis < 0.03f ? -Vec3::XAxis( ) : Vec3::XAxis( ); 
					f32 negation = TxDotXAxis < 0.03f ? -1.0f : 1.0f;

					// Get line intersection result 
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( axis, Vec3::YAxis( ), Vec3::ZAxis( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						// Calculate delta from starting position ( lock to x axis )
						Vec3 u = intersectionResult.mHitPosition - mIntersectionStartPosition; 
 
						// Need to project u onto n
						f32 uDotN = u.Dot( Tx );

						// Store delta as final projection
						mDelta = Tx * uDotN;

						// Reset start position
						mIntersectionStartPosition = mIntersectionStartPosition + mDelta; 

						mDelta *= negation; 

						// Math::Clamp to x axis check 
						mDelta.y = 0.0f; 
						mDelta.z = 0.0f;

						// Accumulate scale delta
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleForwardAxis ) :
				{
					// Find dot between cam forward and right axis
					Vec3 Tz = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );
 
					f32 TzDotZAxis = Tz.Dot( Vec3::ZAxis( ) ); 

					Vec3 axis = TzDotZAxis < 0.03f ? -Vec3::ZAxis( ) : Vec3::ZAxis( ); 
					f32 negation = TzDotZAxis < 0.03f ? -1.0f : 1.0f;

					// Get line intersection result 
					LineIntersectionResult intersectionResult = GetLineIntersectionResult( axis, Vec3::XAxis( ), Vec3::YAxis( ) ); 

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

						// Math::Clamp to z axis check
						mDelta.x = 0.0f; 
						mDelta.y = 0.0f;

						// Accumulate scale delta
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}
				} break;

				case ( TransformWidgetRenderableType::ScaleUpAxis ) :
				{
					Vec3 Ty = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) ).Normalize( );
 
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

						// Math::Clamp to y axis check 
						mDelta.x = 0.0f; 
						mDelta.z = 0.0f;

						mDelta.y = Math::Clamp( mDelta.y, -5.0f, 5.0f );

						// Accumulate scale delta
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}
				} break;

				case ( TransformWidgetRenderableType::ScaleYZAxes ):
				{ 
					// Axis of rotation
					Vec3 Tx = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tx );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Math::Clamp to x axis check
						if ( Tx == Vec3::XAxis( ) )
						{
							mDelta.x = 0.0f; 
						}

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;

						// Accumulate scale delta 
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleXYAxes ):
				{ 
					// Axis of rotation
					Vec3 Tz = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );

					// Get line intersection result with plane
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tz );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Math::Clamp z axis check
						mDelta.z = 0.0f; 

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;

						// Accumulate scale delta 
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleXZAxes ):
				{ 
					// Axis of rotation
					Vec3 Ty = ( mScaleWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3( 0.0f, 1.0f, 0.0f ) ).Normalize( );

					// Get intersection result of plane 
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Ty );

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						const Vec3* hp = &intersectionResult.mHitPosition;

						// Calculate delta from starting position ( lock to xY axes )
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Math::Clamp to y axis check
						mDelta.y = 0.0f; 

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition;

						// Accumulate scale delta 
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}

				} break;

				case ( TransformWidgetRenderableType::ScaleRoot ):
				{
					// Get intersection result
					LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( camera->Forward( ).Normalize( ) ); 

					// Check for intersection hit result
					if ( intersectionResult.mHit )
					{
						// Calculate delta
						const Vec3* hp = &intersectionResult.mHitPosition; 

						// Store delta
						mDelta = intersectionResult.mHitPosition - mIntersectionStartPosition;

						// Screen projected mouse coordinates
						Vec2 mc = mEditorApp->GetSceneViewProjectedCursorPosition( );

						// Store previous position as new intersection position
						mIntersectionStartPosition = intersectionResult.mHitPosition; 

						// Get dot product with camera forward and world x axis
						f32 xDiff = mc.x - mPreviousMouseCoords.x;
						f32 sign = xDiff > 0.0f ? 1.0f : -1.0f ; 

						// Math::Clamp to x axis
						mDelta = Vec3( fabs( mDelta.x ) * sign );

						// Accumulate scale delta 
						mAccumulatedScaleDelta += mDelta; 

						// Set final scale
						transform->SetScale( Vec3::SnapTo( mRootTransform.GetScale( ) + GetAccumulatedScaleDelta( ), mSnapEnabled[ (u32)TransformationMode::Scale ] ? GetScaleSnap( ) : 0.0f ) );
					}

				} break;

				case ( TransformWidgetRenderableType::RotationForwardAxis ) :
				{
					GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );

					// Define plane and get intersection result with ray from mouse
					Vec3 planeNormal = Vec3::ZAxis( );
					LineIntersectionResult intersectionResult = mTransformSpace == TransformSpace::Local ? GetLineIntersectionResultSingleAxis( planeNormal ) : GetLineIntersectionResultSingleAxisOverride( planeNormal ); 

					f32 distFromCam = ( mActiveWidget->GetWorldTransform( ).GetPosition( ).Distance( camera->GetPosition( ) ) );
					f32 denom = distFromCam != 0.0f ? distFromCam: 2.0f;
 
					Vec3 camDir = ( camera->GetPosition( ) - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( );

					if ( intersectionResult.mHit )
					{
						const Vec3 endPositionVector = ( intersectionResult.mHitPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) );
						Vec3 startNormal = ( mIntersectionStartPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( );
						const Vec3 endNormal = ( endPositionVector ).Normalize( );
						const Vec3 immutableStartNormal = ( mImmutableIntersectionStartPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( );
						const Vec3 rotationForward = mActiveWidget->GetWorldTransform( ).GetRotation( ) * Vec3( 0.0f, 0.0f, 1.0f );

						// Start normal
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + immutableStartNormal * 2.0f, Vec3( 1.0f, 0.0f, 0.0f ) );
						// End normal
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + endNormal * endPositionVector.Length( ), Vec3( 0.0f, 1.0f, 0.0f ) );
						// Widget forward
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + rotationForward * 2.0f, Vec3( 0.0f, 0.0f, 1.0f ) );

						f32 length = endPositionVector.Length( ) / denom;
						f32 angle = Math::ToDegrees( startNormal.SignedAngleBetween( endNormal ) );

						if ( length > 1.0f )
						{
							angle *= length;
						} 

						Vec3 cross = startNormal.Cross( endNormal );
						if ( rotationForward.Dot( cross ) < 0.0f )
						{
							angle *= -1.0f;
						} 

						mIntersectionStartPosition = intersectionResult.mHitPosition; 

						mAngleDelta = angle; 
						mAccumulatedAngleDelta += mAngleDelta;
						f32 snapAngle = Vec3::SnapTo( Vec3( mAccumulatedAngleDelta ), mSnapEnabled[ (u32)TransformationMode::Rotation ] ? GetRotationSnap() : 0.0f ).x;
						mDeltaRotation = Quaternion::AngleAxis( Math::ToRadians( snapAngle ), planeNormal );

						switch ( mTransformSpace )
						{
							case TransformSpace::Local: transform->SetRotation( mRootTransform.GetRotation( ) * mDeltaRotation ); break;
							case TransformSpace::World: transform->SetRotation( mDeltaRotation * mRootTransform.GetRotation( ) ); break; 
						} 
					}

				} break; 

				case ( TransformWidgetRenderableType::RotationRightAxis ):
				{
					// Define plane and get intersection result with ray from mouse
					Vec3 planeNormal = Vec3::XAxis( );
					LineIntersectionResult intersectionResult = mTransformSpace == TransformSpace::Local ? GetLineIntersectionResultSingleAxis( planeNormal ) : GetLineIntersectionResultSingleAxisOverride( planeNormal ); 

					f32 distFromCam = ( mActiveWidget->GetWorldTransform( ).GetPosition( ).Distance( camera->GetPosition( ) ) );
					f32 denom = distFromCam != 0.0f ? distFromCam: 2.0f;

					if ( intersectionResult.mHit )
					{
						const Vec3 endPositionVector = ( intersectionResult.mHitPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) );
						const Vec3 startNormal = ( mIntersectionStartPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( );
						const Vec3 endNormal = ( endPositionVector ).Normalize( ); 
						const Vec3 immutableStartNormal = ( mImmutableIntersectionStartPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( );
						const Vec3 rotationRight = mActiveWidget->GetWorldTransform( ).GetRotation( ) * Vec3( 1.0f, 0.0f, 0.0f );

						// Start normal
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + immutableStartNormal * 2.0f, Vec3( 1.0f, 0.0f, 0.0f ) );
						// End normal
						//gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + endNormal * 2.0f, Vec3( 0.0f, 1.0f, 0.0f ) );
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + endNormal * endPositionVector.Length( ), Vec3( 0.0f, 1.0f, 0.0f ) );
						// Widget forward
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + rotationRight * 2.0f, Vec3( 0.0f, 0.0f, 1.0f ) );

						f32 length = endPositionVector.Length( ) / denom;
						f32 angle = Math::ToDegrees( startNormal.SignedAngleBetween( endNormal ) );

						if ( length > 1.0f )
						{
							angle *= length;
						} 

						Vec3 cross = startNormal.Cross( endNormal );
						if ( rotationRight.Dot( cross ) < 0.0f )
						{
							angle *= -1.0f;
						} 

						mIntersectionStartPosition = intersectionResult.mHitPosition; 
 
						mAngleDelta = angle; 
						mAccumulatedAngleDelta += mAngleDelta;
						f32 snapAngle = Vec3::SnapTo( Vec3( mAccumulatedAngleDelta ), mSnapEnabled[ (u32)TransformationMode::Rotation ] ? GetRotationSnap() : 0.0f ).x;
						mDeltaRotation = Quaternion::AngleAxis( Math::ToRadians( snapAngle ), planeNormal );

						switch ( mTransformSpace )
						{
							case TransformSpace::Local: transform->SetRotation( mRootTransform.GetRotation( ) * mDeltaRotation ); break;
							case TransformSpace::World: transform->SetRotation( mDeltaRotation * mRootTransform.GetRotation( ) ); break; 
						} 
					}

				} break;

				case ( TransformWidgetRenderableType::RotationUpAxis ) :
				{
					// Define plane and get intersection result with ray from mouse 
					Vec3 planeNormal = Vec3::YAxis( );
					LineIntersectionResult intersectionResult = mTransformSpace == TransformSpace::Local ? GetLineIntersectionResultSingleAxis( planeNormal ) : GetLineIntersectionResultSingleAxisOverride( planeNormal ); 
					
					f32 distFromCam = ( mActiveWidget->GetWorldTransform( ).GetPosition( ).Distance( camera->GetPosition( ) ) );
					f32 denom = distFromCam != 0.0f ? distFromCam: 2.0f;

					if ( intersectionResult.mHit )
					{
						const Vec3 endPositionVector = ( intersectionResult.mHitPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) );
						const Vec3 startNormal = ( mIntersectionStartPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( );
						const Vec3 endNormal = ( endPositionVector ).Normalize( ); 
						const Vec3 immutableStartNormal = ( mImmutableIntersectionStartPosition - mActiveWidget->GetWorldTransform( ).GetPosition( ) ).Normalize( ); 
						const Vec3 rotationUp = mActiveWidget->GetWorldTransform( ).GetRotation( ) * Vec3( 0.0f, 1.0f, 0.0f );

						// Start normal
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + immutableStartNormal * 2.0f, Vec3( 1.0f, 0.0f, 0.0f ) );
						// End normal
						//gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + endNormal * 2.0f, Vec3( 0.0f, 1.0f, 0.0f ) );
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + endNormal * endPositionVector.Length( ), Vec3( 0.0f, 1.0f, 0.0f ) );
						// Widget forward
						gfx->DrawDebugLine( mActiveWidget->GetWorldTransform( ).GetPosition( ), mActiveWidget->GetWorldTransform( ).GetPosition( ) + rotationUp * 2.0f, Vec3( 0.0f, 0.0f, 1.0f ) );

						f32 length = endPositionVector.Length( ) / denom;
						f32 angle = Math::ToDegrees( startNormal.SignedAngleBetween( endNormal ) );

						if ( length > 1.0f )
						{
							angle *= length;
						} 

						Vec3 cross = startNormal.Cross( endNormal );
						if ( rotationUp.Dot( cross ) < 0.0f )
						{
							angle *= -1.0f;
						} 

						mIntersectionStartPosition = intersectionResult.mHitPosition; 

						mAngleDelta = angle; 
						mAccumulatedAngleDelta += mAngleDelta;
						f32 snapAngle = Vec3::SnapTo( Vec3( mAccumulatedAngleDelta ), mSnapEnabled[ (u32)TransformationMode::Rotation ] ? GetRotationSnap() : 0.0f ).x;
						mDeltaRotation = Quaternion::AngleAxis( Math::ToRadians( snapAngle ), planeNormal );

						switch ( mTransformSpace )
						{
							case TransformSpace::Local: transform->SetRotation( mRootTransform.GetRotation( ) * mDeltaRotation ); break;
							case TransformSpace::World: transform->SetRotation( mDeltaRotation * mRootTransform.GetRotation( ) ); break; 
						} 
					}
				} break;

			} 

			// Store previous mouse coordinates
			//mPreviousMouseCoords = input->GetMouseCoords( );
			mPreviousMouseCoords = mEditorApp->GetSceneViewProjectedCursorPosition( );

			// Set position and rotation of widget based on final transform
			SetPosition( transform->GetPosition( ) );
			SetRotation( transform->GetRotation( ) );
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
		Input* input = EngineSubsystem( Input );

		if ( result.mHit )
		{
			const Vec3* hp = &result.mHitPosition; 
			// Store position and then store widget that's being operated upon
			mInteractingWithTransformWidget = true;
			mType = type;
			mIntersectionStartPosition = result.mHitPosition;
			mImmutableIntersectionStartPosition = result.mHitPosition;
			mRootStartPosition = mActiveWidget->GetWorldTransform().GetPosition( );
			mPreviousMouseCoords = mEditorApp->GetSceneViewProjectedCursorPosition( );
			mStartRotation = mActiveWidget->GetWorldTransform( ).GetRotation( );
			mRootStartScale = mActiveWidget->GetWorldTransform( ).GetScale( );
		} 
	} 

	void EditorTransformWidget::BeginInteraction( TransformWidgetRenderableType type, const Transform& transform )
	{
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		Input* input = EngineSubsystem( Input );
		Camera* camera = gfx->GetGraphicsSceneCamera( )->ConstCast< Camera >();

		// Store transform to interact with
		mRootTransform = transform; 

		// Begin interaction with active widget
		if ( mActiveWidget )
		{
			mActiveWidget->BeginInteraction( type );
		}

		// Reset accumulated deltas
		mAccumulatedTranslationDelta = Vec3( 0.0f );
		mAccumulatedScaleDelta = Vec3( 0.0f );
		mAccumulatedRotationDelta = Quaternion( );
		mAccumulatedAngleDelta = 0.0f;

		// Look for picked transform widget
		switch ( type )
		{ 
			case ( TransformWidgetRenderableType::TranslationRoot ):
			{
				// Axis of rotation
				Vec3 cF = camera->Forward( ).Normalize( ); 

				// Do intersection test
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( cF );

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
				Vec3 Tz = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation() * Vec3::ZAxis() ).Normalize(); 

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tz );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationXYAxes ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationXZAxes ):
			{ 
				// Axis of rotation
				Vec3 Ty = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::YAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Ty );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::TranslationXZAxes ); 
			} break;

			case ( TransformWidgetRenderableType::TranslationYZAxes ):
			{ 
				// Axis of rotation
				Vec3 Tx = ( mTranslationWidget.mRoot.mWorldTransform.GetRotation( ) * Vec3::XAxis( ) ).Normalize( ); 

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tx );

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
				Vec3 Tx = ( mActiveWidget->GetWorldTransform( ).GetRotation( ) * Vec3::XAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tx );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleYZAxes ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleXYAxes ):
			{ 
				// Axis of rotation
				Vec3 Tz = ( mActiveWidget->GetWorldTransform( ).GetRotation( ) * Vec3::ZAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Tz );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleXYAxes ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleXZAxes ):
			{ 
				// Axis of rotation
				Vec3 Ty = ( mActiveWidget->GetWorldTransform( ).GetRotation( ) * Vec3::YAxis( ) ).Normalize( );

				// Get intersection result of plane 
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( Ty );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleXZAxes ); 
			} break;

			case ( TransformWidgetRenderableType::ScaleRoot ):
			{
				// Axis of rotation
				Vec3 cF = camera->Forward( ).Normalize( ); 

				// Do intersection test
				LineIntersectionResult intersectionResult = GetLineIntersectionResultSingleAxisOverride( cF );

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::ScaleRoot ); 
			} break;

			case ( TransformWidgetRenderableType::RotationForwardAxis ) :
			{
				// Get line intersection result
				LineIntersectionResult intersectionResult = mTransformSpace == TransformSpace::Local ? GetLineIntersectionResultSingleAxis( Vec3::ZAxis() ) : GetLineIntersectionResultSingleAxisOverride( Vec3::ZAxis() ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::RotationForwardAxis ); 
			} break; 

			case ( TransformWidgetRenderableType::RotationRightAxis ) :
			{
				// Get line intersection result
				LineIntersectionResult intersectionResult = mTransformSpace == TransformSpace::Local ? GetLineIntersectionResultSingleAxis( Vec3::XAxis() ) : GetLineIntersectionResultSingleAxisOverride( Vec3::XAxis() ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::RotationRightAxis ); 
			} break; 

			case ( TransformWidgetRenderableType::RotationUpAxis ) :
			{
				// Get line intersection result
				LineIntersectionResult intersectionResult = mTransformSpace == TransformSpace::Local ? GetLineIntersectionResultSingleAxis( Vec3::YAxis() ) : GetLineIntersectionResultSingleAxisOverride( Vec3::YAxis() ); 

				// Store information
				StoreIntersectionResultInformation( intersectionResult, TransformWidgetRenderableType::RotationUpAxis ); 
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
 
	f32 EditorTransformWidget::GetAngleDelta( ) const
	{
		return mAngleDelta;
	}

	Quaternion EditorTransformWidget::GetDeltaRotation( ) const
	{
		return mDeltaRotation;
	}

	TransformWidgetRenderableType EditorTransformWidget::GetInteractedWidgetType( )
	{
		return mType;
	}

	bool EditorTransformWidget::IsValidID( const u32& id )
	{
		s32 value = ( s32 )id - MAX_ENTITIES;
		if ( value >= 0 && value < ( s32 )TransformWidgetRenderableType::Count )
		{
			return true;
		}

		return false;
	} 

	bool EditorTransformWidget::IsSnapEnabled( const TransformationMode& mode )
	{
		return mSnapEnabled[ ( u32 )mode ];
	}

	void EditorTransformWidget::EnableSnapping( bool enable, const TransformationMode& mode )
	{
		mSnapEnabled[ ( u32 )mode ] = enable;
	}

	Vec3 EditorTransformWidget::GetSnapSettings( ) const
	{
		return mSnapSettings;
	}


	void EditorTransformWidget::SetSnapSettings( const Vec3& snap )
	{
		mSnapSettings = snap;
	}

	f32 EditorTransformWidget::GetTranslationSnap( ) const
	{
		return mSnapSettings.x;
	}

	f32 EditorTransformWidget::GetRotationSnap( ) const
	{
		return mSnapSettings.y;
	}

	f32 EditorTransformWidget::GetScaleSnap( ) const
	{
		return mSnapSettings.z;
	} 

	void EditorTransformWidget::SetTranslationSnap( const f32& snap )
	{
		mSnapSettings.x = Math::Max( snap, 0.0f );
	}

	void EditorTransformWidget::SetRotationSnap( const f32& snap )
	{
		mSnapSettings.y = Math::Max( snap, 0.0f );
	}

	void EditorTransformWidget::SetScaleSnap( const f32& snap )
	{
		mSnapSettings.z = Math::Max( snap, 0.0f );
	} 

	Vec3 EditorTransformWidget::GetIntersectionStartPosition( ) const
	{
		return mIntersectionStartPosition;
	}

	Vec3 EditorTransformWidget::GetAccumulatedTranslationDelta( ) const
	{
		return mAccumulatedTranslationDelta;
	}

	Vec3 EditorTransformWidget::GetRootStartPosition( ) const
	{
		return mRootStartPosition;
	}

	Vec3 EditorTransformWidget::GetRootStartScale( ) const
	{
		return mRootStartScale;
	}

	Vec3 EditorTransformWidget::GetAccumulatedScaleDelta( ) const 
	{
		return mAccumulatedScaleDelta;
	}
}
