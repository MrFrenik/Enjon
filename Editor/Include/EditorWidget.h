// @file EditorWidget.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_WIDGET_H
#define ENJON_EDITOR_WIDGET_H

#include <Graphics/Renderable.h>

namespace Enjon
{
	class EditorTransformWidget;

	enum class TransformWidgetRenderableType
	{
		TranslationRoot,
		TranslationForwardAxis,
		TranslationUpAxis,
		TranslationRightAxis,
		TranslationXYAxes,
		TranslationXZAxes,
		TranslationYZAxes,
		ScaleRoot,
		ScaleForwardAxis,
		ScaleUpAxis,
		ScaleRightAxis,
		ScaleXYAxes,
		ScaleXZAxes,
		ScaleYZAxes,
		RotationForwardAxis,
		RotationRightAxis,
		RotationUpAxis,
		Count
	};

	class TransformHeirarchy : public Object
	{ 
		public:
			TransformHeirarchy( ) = default;
			~TransformHeirarchy( )
			{
				mChildren.clear( );
			}

			bool HasParent( )
			{
				return mParent != nullptr;
			}

			void CalculateWorldTransform()
			{ 
				// WorldScale = ParentScale * LocalScale
				// WorldRot = LocalRot * ParentRot
				// WorldPos = ParentPos + [ ParentRot * ( ParentScale * LocalPos ) ]

				if ( !HasParent( ) )
				{
					mWorldTransform = mLocalTransform;
					return;
				}

				// Get parent transform recursively
				//Transform parent = mParent->mWorldTransform;

				//Enjon::Vec3 worldScale = parent.GetScale() * mLocalTransform.GetScale();
				//Enjon::Quaternion worldRot = ( parent.GetRotation() * mLocalTransform.GetRotation() ).Normalize( );
				//Enjon::Vec3 worldPos = parent.GetPosition() + ( parent.GetRotation().Normalize() * ( parent.GetScale() * mLocalTransform.GetPosition() ) );

				mWorldTransform = mLocalTransform * mParent->mWorldTransform;

				//mWorldTransform = Transform( worldPos, worldRot, worldScale ); 
			}

			void CalculateLocalTransform( ) 
			{
				// RelScale = WorldScale / ParentScale 
				// RelRot	= Inverse(ParentRot) * WorldRot
				// Trans	= [Inverse(ParentRot) * (WorldPos - ParentPosition)] / ParentScale;

				if ( HasParent( ) )
				{ 
					//Transform parentTransform = mParent->mWorldTransform;
					//Enjon::Quaternion parentInverse = parentTransform.GetRotation().Inverse( ).Normalize(); 

					//Vec3 relativeScale = mWorldTransform.GetScale() / parentTransform.GetScale();
					//Quaternion relativeRot = ( parentInverse * mWorldTransform.GetRotation() ).Normalize();
					//Vec3 relativePos = ( parentInverse * ( mWorldTransform.GetPosition() - parentTransform.GetPosition() ) ) / parentTransform.GetScale(); 

					//mLocalTransform = Transform( relativePos, relativeRot, relativeScale );

					mLocalTransform = mWorldTransform / mParent->mWorldTransform;
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

	class TransformRenderableHeirarchy : public TransformHeirarchy
	{
		public:
			TransformRenderableHeirarchy( ) = default;
			~TransformRenderableHeirarchy( ) = default;

			Renderable mRenderable;
	}; 

	class EditorWidget
	{
		public:
			EditorWidget( ) = default;
			~EditorWidget( )
			{
				mTransformHeirarchies.clear( );
			}

			virtual void Update( );
			virtual void Initialize( EditorTransformWidget* owner ) = 0;
			virtual void BeginInteraction( TransformWidgetRenderableType type ) = 0;
			virtual void Interact( ) = 0;
			virtual void EndInteraction( TransformWidgetRenderableType type ) = 0; 

			virtual void Enable( );
			virtual void Disable( ); 

			Transform GetWorldTransform( );
			void SetTransform( const Transform& transform ); 
			void SetPosition( const Vec3& position ); 
			void SetScale( const f32& scale ); 
			void SetRotation( const Quaternion& rotation );
			void SetRotation( const Vec3& eulerAngles );

		protected:
			EditorTransformWidget* mOwner = nullptr;
			TransformHeirarchy* mRootHeirarchy = nullptr; 
			Vector<TransformRenderableHeirarchy*> mTransformHeirarchies;
	};
}

#endif
