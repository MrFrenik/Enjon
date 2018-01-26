// @file EditorWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_WIDGET_H
#define ENJON_EDITOR_WIDGET_H

#include <Graphics/Renderable.h>

namespace Enjon
{
	enum class TransformWidgetRenderableType
	{
		TranslationRoot,
		TranslationForwardAxis,
		TranslationUpAxis,
		TranslationRightAxis,
		TranslationXYAxes,
		TranslationXZAxes,
		TranslationYZAxes
	};

	ENJON_CLASS( )
	class TransformHeirarchy : public Object
	{
		ENJON_CLASS_BODY( )

		public:
			TransformHeirarchy( ) = default;
			~TransformHeirarchy( ) = default;

			bool HasParent( )
			{
				return mParent != nullptr;
			}

			void CalculateWorldTransform()
			{ 
				// WorldScale = ParentScale * LocalScale
				// WorldRot = LocalRot * ParentRot
				// WorldPos = ParentPos + [ Inverse(ParentRot) * ( ParentScale * LocalPos ) ]

				if ( !HasParent( ) )
				{
					mWorldTransform = mLocalTransform;
					return;
				}

				// Get parent transform recursively
				Transform parent = mParent->mWorldTransform;

				Enjon::Vec3 worldScale = parent.Scale * mLocalTransform.Scale;
				Enjon::Quaternion worldRot = ( mLocalTransform.Rotation * parent.Rotation ).Normalize( );
				Enjon::Vec3 worldPos = parent.Position + ( parent.Rotation.Inverse().Normalize() * ( parent.Scale * mLocalTransform.Position ) );

				mWorldTransform = Transform( worldPos, worldRot, worldScale ); 
			}

			void CalculateLocalTransform( ) 
			{
				// RelScale = WorldScale / ParentScale 
				// RelRot	= Inverse(ParentRot) * WorldRot
				// Trans	= [Inverse(ParentRot) * (WorldPos - ParentPosition)] / ParentScale;

				if ( HasParent( ) )
				{ 
					Transform parentTransform = mParent->mWorldTransform;
					Enjon::Quaternion parentInverse = parentTransform.Rotation.Inverse( ).Normalize(); 

					Vec3 relativeScale = mWorldTransform.Scale / parentTransform.Scale;
					Quaternion relativeRot = ( parentInverse * mWorldTransform.Rotation ).Normalize();
					Vec3 relativePos = ( parentInverse * ( mWorldTransform.Position - parentTransform.Position ) ) / parentTransform.Scale;

					mLocalTransform = Transform( relativePos, relativeRot, relativeScale );
				}
			}

			void AddChild( TransformHeirarchy* child )
			{ 
				if ( child == nullptr )
				{
					return;
				}

				// Set parent to this
				child->SetParent( this );

				// Make sure child doesn't exist in vector before pushing back
				auto query = std::find(mChildren.begin(), mChildren.end(), child);
				if (query == mChildren.end())
				{
					// Add child to children list
					mChildren.push_back(child);

					// Calculate its world transform with respect to parent
					child->CalculateWorldTransform();
				}
				else
				{
					// Log a warning message here
				} 
			}

			void SetParent( TransformHeirarchy* parent )
			{ 
				// Calculate world transform ( No parent yet, so set world to local )
				CalculateWorldTransform( ); 
				
				// Set parent to this
				mParent = parent;
				
				// Calculate local transform relative to parent
				CalculateLocalTransform( ); 
			}

			ENJON_PROPERTY( )
			Transform mLocalTransform;

			Transform mWorldTransform;
			TransformHeirarchy* mParent = nullptr;
			Vector<TransformHeirarchy*> mChildren;
	};

	ENJON_CLASS( )
	class TransformRenderableHeirarchy : public TransformHeirarchy
	{
		ENJON_CLASS_BODY( )

		public:
			TransformRenderableHeirarchy( ) = default;
			~TransformRenderableHeirarchy( ) = default;

			Renderable mRenderable;
	}; 
}

#endif
