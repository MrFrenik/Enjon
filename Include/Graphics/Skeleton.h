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
#include "Math/Vec4.h" 

#define ENJON_MAX_NUM_JOINTS_PER_VERTEX		4

namespace Enjon
{
	class Skeleton;

	class Joint
	{ 
		friend Skeleton;

		public: 

			/*
			* @brief Constructor
			*/
			Joint( );

			/*
			* @brief Constructor
			*/
			~Joint( );

		protected:
			Vector<u32>		mChildren;
			u32				mID;
			Transform		mInverseBindTransform;
			String			mName;
	};

	struct SkeletalVertexData
	{ 
		Vec3	mPosition;
		Vec3	mNormal;
		Vec3	mTangent;
		Vec2	mUV;
		u32		mJointIndices[ENJON_MAX_NUM_JOINTS_PER_VERTEX];
		f32		mJointWeights[ENJON_MAX_NUM_JOINTS_PER_VERTEX];
	};

	class Skeleton
	{ 
		public:

			/*
			* @brief
			*/
			Skeleton( );

			/*
			* @brief
			*/
			~Skeleton( ); 

		private: 
			u32							mRootID;
			Vector< Joint >				mBones;
			HashMap< String, u32 >		BoneNameLookup;
	};

	ENJON_CLASS( Construct )
	class SkeletalSubMesh : public SubMesh
	{
		friend MeshAssetLoader;
		friend Mesh;

		ENJON_CLASS_BODY( )

	public:
		SkeletalSubMesh( ) = default;

		~SkeletalSubMesh( ) = default;

	private:
		Vector< SkeletalVertexData > mSkeletalVertexData ; 
	}; 

	class SkeletalMesh : public Mesh
	{ 
		public:

			/*
			* @brief
			*/
			SkeletalMesh( );

			/*
			* @brief
			*/
			~SkeletalMesh( );

		private: 
			Vector< SkeletalSubMesh > mSkeletalSubmeshes;
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