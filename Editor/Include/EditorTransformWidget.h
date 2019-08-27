// @file EditorTransformWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_TRANSFORM_WIDGET_H
#define ENJON_EDITOR_TRANSFORM_WIDGET_H
 
#include "EditorWidget.h"
#include "EditorTranslationWidget.h"
#include "EditorScaleWidget.h"
#include "EditorRotationWidget.h"

#include <Graphics/Renderable.h>

namespace Enjon
{ 
	class EditorApp;
	class EditorViewport;

	ENJON_ENUM( )
	enum class TransformationMode
	{
		Translation,
		Scale,
		Rotation,
		Count
	};

	ENJON_CLASS( )
	class EditorTransformWidget : public Object
	{ 
		ENJON_CLASS_BODY( EditorTransformWidget )

		public: 
			void Initialize( EditorViewport* vp );
			void Update( );

			Transform GetWorldTransform( );

			void SetPosition( const Vec3& position );
			void SetScale( const f32& scale );
			void SetRotation( const Quaternion& rotation );
			void SetRotation( const Vec3& eulerAngles );

			EditorTranslationWidget mTranslationWidget;
			EditorScaleWidget mScaleWidget;
			EditorRotationWidget mRotationWidget;

			void BeginInteraction( TransformWidgetRenderableType type, const Transform& transform );
			void InteractWithWidget( Transform* transform );
			void EndInteraction( );
			TransformWidgetRenderableType GetInteractedWidgetType( );
			TransformationMode GetTransformationMode( ); 
			TransformSpace GetTransformSpace( ) const;

			void SetTransformSpace( TransformSpace space );

			bool IsInteractingWithWidget( ) const;
			Vec3 GetDelta( ) const; 
			f32 GetAngleDelta( ) const;
			Quaternion GetDeltaRotation( ) const;

			void SetTransformationMode( TransformationMode mode ); 

			void Enable( bool enable );

			static bool IsValidID( const u32& id ); 

			void EnableSnapping( bool enable, const TransformationMode& mode );
			bool IsSnapEnabled( const TransformationMode& mode );

			Vec3 GetSnapSettings( ) const;
			void SetSnapSettings( const Vec3& snap );

			f32 GetTranslationSnap( ) const;
			f32 GetScaleSnap( ) const;
			f32 GetRotationSnap( ) const;

			void SetTranslationSnap( const f32& snap );
			void SetScaleSnap( const f32& snap );
			void SetRotationSnap( const f32& snap );

			Vec3 GetIntersectionStartPosition( ) const;
			Vec3 GetAccumulatedTranslationDelta( ) const;
			Vec3 GetRootStartPosition( ) const;
			Vec3 GetRootStartScale( ) const;
			Vec3 GetAccumulatedScaleDelta( ) const; 

		private:
			LineIntersectionResult GetLineIntersectionResult( const Vec3& axisA, const Vec3& axisB, const Vec3& axisC, bool comparedSupportingAxes = true, bool overrideAxis = false, const Vec3& axisToUseAsPlaneNormal = Vec3(0.0f) );
			LineIntersectionResult GetLineIntersectionResultSingleAxis( const Vec3& axis );
			LineIntersectionResult GetLineIntersectionResultSingleAxisOverride( const Vec3& axis );
			void StoreIntersectionResultInformation( const LineIntersectionResult& result, TransformWidgetRenderableType type ); 

		private:
			EditorViewport* mViewport = nullptr;
			EditorWidget* mActiveWidget = nullptr;
			Transform mWorldTransform;					
			TransformationMode mMode = TransformationMode::Translation;
			TransformWidgetRenderableType mType = TransformWidgetRenderableType::TranslationRightAxis;
			Vec3 mIntersectionStartPosition;
			Vec3 mImmutableIntersectionStartPosition;
			Vec3 mRootStartPosition;
			Vec3 mRootStartScale;
			Quaternion mStartRotation;
			Vec3 mDelta;
			f32 mAngleDelta;
			Quaternion mDeltaRotation;
			Vec2 mPreviousMouseCoords;
			bool mInteractingWithTransformWidget = true; 
			bool mEnabled = false;
			bool mSetPreviousAngle = false;
			f32 mPreviousAngle = 0.0f;

			bool mSnapEnabled[ ( u32 )TransformationMode::Count ];

			// X = Translation, Y = Rotation ( Degrees ), Z = Scale
			Vec3 mSnapSettings = Vec3( 1.0f, 10.0f, 1.0f );
			Vec3 mAccumulatedTranslationDelta = Vec3( 0.0f );
			Vec3 mAccumulatedScaleDelta = Vec3( 0.0f );
			Quaternion mAccumulatedRotationDelta = Quaternion( );
			f32 mAccumulatedAngleDelta = 0.0f;

			Transform mRootTransform;

			TransformSpace mTransformSpace = TransformSpace::World; 
	}; 
}

#endif
