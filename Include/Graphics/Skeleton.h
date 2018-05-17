// @file Skeleton.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SKELETON_H
#define ENJON_SKELETON_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"
#include "Math/Maths.h"
#include "Asset/Asset.h" 
#include "Graphics/Mesh.h"

#define ENJON_MAX_NUM_JOINTS_PER_VERTEX		4

namespace Enjon
{
	// Forward Declarations
	class SkeletalAnimation;
	class SkeletalMeshAssetLoader;
	class SkeletonAssetLoader;
	class SkeletalAnimationComponent;
	class Skeleton;

	ENJON_CLASS( )
	class Joint : public Object
	{ 
		ENJON_CLASS_BODY( Joint )

		friend SkeletonAssetLoader;
		friend SkeletalMeshAssetLoader;
		friend Skeleton;

		public: 
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

		protected:
			ENJON_PROPERTY( HideInEditor ) 
			s32					mParentID;

			ENJON_PROPERTY( HideInEditor )
			Vector< u32 >		mChildren;

			ENJON_PROPERTY( HideInEditor )
			u32					mID;

			ENJON_PROPERTY( HideInEditor )
			Mat4x4				mInverseBindMatrix;

			ENJON_PROPERTY( HideInEditor )
			String				mName;
	}; 

	struct VertexJointData
	{
		VertexJointData( )
		{
			// Init data
			for ( u32 i = 0; i < ENJON_MAX_NUM_JOINTS_PER_VERTEX; ++i )
			{
				mWeights[i] = 0.0f;
				mIDS[i] = 0;
			}
		}

		f32 mWeights[ENJON_MAX_NUM_JOINTS_PER_VERTEX];
		u32 mIDS[ENJON_MAX_NUM_JOINTS_PER_VERTEX];
	};

	ENJON_CLASS( )
	class Skeleton : public Asset
	{ 
		ENJON_CLASS_BODY( Skeleton )

		friend SkeletonAssetLoader;
		friend SkeletalMeshAssetLoader;
		friend SkeletalAnimationComponent;

		public: 

			/*
			* @brief
			*/
			bool HasJoint( const String& name ) const;

			/*
			* @brief
			*/
			Vector< Mat4x4 > GetTransforms( const AssetHandle< SkeletalAnimation >& animation, const f32& time ) const; 

			/*
			* @brief
			*/
			u32 GetNumberOfJoints( ) const;

			/*
			* @brief
			*/
			s32 GetJointIndex( const String& name ) const;

			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override;

			/*
			* @brief
			*/
			s32 GetRootID( ) const;

		protected:

			/*
			* @brief
			*/
			void CalculateTransform( const u32& jointID, const Mat4x4& parentMatrix, Vector<Mat4x4>& outMatrices, const SkeletalAnimation* animation, const f32& time ) const;

		protected: 
			ENJON_PROPERTY( HideInEditor )
			s32							mRootID = -1;

			ENJON_PROPERTY( HideInEditor )
			Vector< Joint >				mJoints;

			ENJON_PROPERTY( HideInEditor )
			HashMap< String, u32 >		mJointNameLookup;
 
			ENJON_PROPERTY( HideInEditor )
			Mat4x4						mGlobalInverseTransform; 
	};
}

#endif


/*
	// Generic vertex data definition

	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 4, 4, GL_UNSIGNED_INT, GL_FALSE, sizeof( SkeletalVertexData ), (void*)offsetof( SkeletalVertexData, mJointIndices ) );

	// Joint Weights
	glEnableVertexAttribArray( 5 );
	glVertexAttribPointer( 5, 4, GL_FLOAT, GL_FALSE, sizeof( SkeletalVertexData ), (void*)offsetof( SkeletalVertexData, mJointWeights ) );

	enum class VertexAttributeFormat
	{
		Float4,
		Float3,
		Float2,
		Float,
		UnsignedInt4,
		UnsignedInt3,
		UnsignedInt2,
		UnsignedInt
	};

	// How do you actually upload the vertex data? Submesh void* data? Data will need to be interleaved... So just having arrays of data might not work well... Or could it?

	// Data as byte buffer stream?
	ByteBuffer mData;

	Just have Vector< f32 > mData ? 

	class VertexDataDeclaration
	{ 
		public: 
			usize GetSizeInBytes(); 

			s32 GetOffset( const u32& attributeIndex );

		private:
			void CalculateSizeInBytes(); 
		
		private:
			usize mSizeInBytes;
			Vector< f32 >	mFloatData;
			Vector< u32 >	mUnsignedIntData; 
			Vector< u8 >	mUnsignedByteData; 
			Vector< VertexAttributeFormat > mVertexAttributes;
	};

	s32 VertexDataDeclaration::ByteOffset( const u32& attributeIndex )
	{
		// Error check
		if ( attributeIndex >= mVertexAttributes.size() )
		{
			return -1;
		}

		// Recursively calculate offset
		s32 totalOffset = 0;
		for ( u32 i = 0; i < attributeIndex )
		{ 
			totalOffset += ByteOffset( i );
		}

		return totalOffset;
	}

	usize VertexDataDeclaration::GetSizeInBytes()
	{
		return mSizeInBytes;
	}

	void VertexDataDeclaration::CalculateSizeInBytes()
	{
		usize sz = 0;
		for ( auto& format : mVertexAttributes )
		{
			switch ( format )
			{
				case VertexAttributeFormat::Float4:
				{
					sz += 4 * sizeof( f32 );
				} break; 

				case VertexAttributeFormat::Float3:
				{
					sz += 3 * sizeof( f32 );
				} break;

				...

				case VertexAttributeFormat::UnsignedInt4:
				{
					sz += 3 * sizeof( u32 );
				} brek;

				...
			} 
		}

		// Set size in bytes
		mSizeInBytes = sz;
	}

	VertexDataDeclaration::Add( const VertexAttributeFormat& format )
	{
		switch ( format )
		{
			case VertexAttributeFormat::Float4:
			{
				mDecl.push_back( format );
			} break;
			...
		}

		// Recalculate total size in bytes of this declaration
		CalculateSizeInBytes();
	} 

	// Example - Skeletal submesh declaration
	VertexDataDeclaration vertexDecl;
	vertexDecl.Add( VertexDataDeclaration::Float3 );			// Position
	vertexDecl.Add( VertexDataDeclaration::Float3 );			// Normal
	vertexDecl.Add( VertexDataDeclaration::Float3 );			// Tangent
	vertexDecl.Add( VertexDataDeclaration::Float2 );			// UV
	vertexDecl.Add( VertexDataDeclaration::UnsignedInt4 );		// Joint Indices
	vertexDecl.Add( VertexDataDeclaration::Float4 );			// Joint Weights 

	Submesh( vertexDecl, vertexData ); 

	// Should be able to pass this definition into a submesh constructor and then handle creation of the submesh
	Submesh( const VertexDataDeclaration& vertexDecl, const ByteBuffer& vertexData )
	{
		// Mesh to construct
		Submesh mesh; 

		// Create and upload mesh data
		glGenBuffers( 1, &sm.mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, sm.mVBO );
		glBufferData( GL_ARRAY_BUFFER, vertexData.GetSize(), vertexData.GetData(), GL_STATIC_DRAW );

		// Grab total size in bytes for data declaration
		usize vertexDeclSize = vertexDecl.GetTotalSizeInBytes(); 
 
		// Vertex attributes
		for ( u32 i = 0; i < vertexDecl.GetAttributes() )
		{
			VertexAttributeFormat attribute = vertexDecl.GetAttributes().at( i );

			// Enable the vertex attribute array
			glEnableVertexAttribArray( i );

			// Bind vertex attributes
			switch ( attribute )
			{
				case VertexAttributeFormat::Float4:
				{ 
					glVertexAttribPointer( i, 4, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)attribute.ByteOffset() ); 
				} break;
				
				case VertexAttributeFormat::UnsignedInt4: 
				{
					glVertexAttribPointer( i, 4, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)attribute.ByteOffset() ); 
				} break;
			}
		} 
	}

*/