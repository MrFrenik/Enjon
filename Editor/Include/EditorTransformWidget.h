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

	enum class TransformationMode
	{
		Translation,
		Scale,
		Rotation
	};

	class EditorTransformWidget
	{ 
		public:
			EditorTransformWidget( ) = default;
			~EditorTransformWidget( ) = default;

			void Initialize( EditorApp* editor );
			void Update( );

			Transform GetWorldTransform( );

			void SetPosition( const Vec3& position );
			void SetScale( const f32& scale );
			void SetRotation( const Quaternion& rotation );

			EditorTranslationWidget mTranslationWidget;
			EditorScaleWidget mScaleWidget;
			EditorRotationWidget mRotationWidget;

			void BeginWidgetInteraction( TransformWidgetRenderableType type );
			void InteractWithWidget( );
			void EndInteraction( );
			TransformWidgetRenderableType GetInteractedWidgetType( );
			TransformationMode GetTransformationMode( ); 

			bool IsInteractingWithWidget( ) const;
			Vec3 GetDelta( ) const; 
			f32 GetAngleDelta( ) const;
			Quaternion GetDeltaRotation( ) const;

			void SetTransformationMode( TransformationMode mode ); 

			void Enable( bool enable );

			static bool IsValidID( const u32& id );

		private:
			LineIntersectionResult GetLineIntersectionResult( const Vec3& axisA, const Vec3& axisB, const Vec3& axisC, bool comparedSupportingAxes = true, const Vec3& axisToUseAsPlaneNormal = Vec3(0.0f) );
			LineIntersectionResult GetLineIntersectionResultSingleAxis( const Vec3& axis );
			void StoreIntersectionResultInformation( const LineIntersectionResult& result, TransformWidgetRenderableType type ); 

		private:
			EditorApp* mEditorApp = nullptr;
			EditorWidget* mActiveWidget = nullptr;
			Transform mWorldTransform;					
			TransformationMode mMode = TransformationMode::Translation;
			TransformWidgetRenderableType mType = TransformWidgetRenderableType::TranslationRightAxis;
			Vec3 mIntersectionStartPosition;
			Vec3 mImmutableIntersectionStartPosition;
			Vec3 mRootStartPosition;
			Vec3 mDelta;
			f32 mAngleDelta;
			Quaternion mDeltaRotation;
			Vec2 mPreviousMouseCoords;
			bool mInteractingWithTransformWidget = true; 
			bool mEnabled = false;
	}; 
}

#endif
