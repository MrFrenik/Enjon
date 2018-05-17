#pragma once
#ifndef ENJON_MESH_H
#define ENJON_MESH_H

#include "System/Types.h"
#include "Defines.h"
#include "Math/Maths.h"
#include "Graphics/Vertex.h"
#include "Asset/Asset.h" 

namespace Enjon 
{ 
	// Forward Declarations
	class SkeletalMeshAssetLoader;
	class SubMesh;
	class Mesh;
	class MeshAssetLoader; 

	ENJON_ENUM( )
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

	ENJON_CLASS( )
	class VertexDataDeclaration : public Object
	{
		friend SkeletalMeshAssetLoader;
		friend MeshAssetLoader;
		friend SubMesh;
		friend Mesh;

		ENJON_CLASS_BODY( VertexDataDeclaration )

		public: 

			/*
			* @brief
			*/
			usize GetSizeInBytes( ) const;

			/*
			* @brief
			*/
			s32 GetByteOffset( const u32& attributeIndex ) const; 


			/*
			* @brief
			*/
			void Add( const VertexAttributeFormat& format ); 

		protected:

			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override;

		private: 

			void CalculateSizeInBytes( );

		protected: 
			usize mSizeInBytes = 0;
			Vector< VertexAttributeFormat > mDecl; 
	};


	struct Vert 
	{
		float Position[3];
		float Normals[3];
		float Tangent[3];
		float UV[2];	
	}; 

	ENJON_CLASS( )
	class SubMesh : public Object
	{ 
		friend SkeletalMeshAssetLoader;
		friend MeshAssetLoader;
		friend Mesh;

		ENJON_CLASS_BODY( SubMesh )

		public:

			/*
			* @brief Constructor
			*/
			SubMesh( Mesh* mesh );

			/*
			* @brief Explicit Destructor
			*/
			virtual void ExplicitDestructor( ) override;

			/*
			* @brief
			*/
			void Bind() const;

			/*
			* @brief
			*/
			void Unbind() const;

			/* 
			* @brief 
			*/
			void Submit() const; 

			/*
			* @brief
			*/
			u32 GetVertexCount( ) const;

			/*
			* @brief
			*/
			u32 GetDrawCount( ) const; 

			/*
			* @brief
			*/
			u32 GetVAO( ) const;

			/*
			* @brief
			*/
			u32 GetVBO( ) const;

			/*
			* @brief
			*/
			u32 GetIBO( ) const;

		protected: 

			/*
			* @brief
			*/
			Result Release( ); 

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
			Vector< Vert > mVerticies; 
			Vector< u32 > mIndicies;	

			// Owning mesh
			Mesh* mMesh = nullptr; 
			ByteBuffer mVertexData;

			GLenum mDrawType;
			GLint mDrawStart = 0;
			GLint mDrawCount = 0;
			GLuint mVAO = 0;
			GLuint mVBO = 0;
			GLuint mIBO = 0;
	}; 

	ENJON_CLASS( )
	class Mesh : public Asset
	{
		friend MeshAssetLoader;

		ENJON_CLASS_BODY( Mesh )

		public:

			/*
			* @brief Explicit Destructor
			*/
			virtual void ExplicitDestructor( ) override;

			/*
			* @brief
			*/
			const Vector<SubMesh*>& GetSubmeshes( ) const;

			/*
			* @brief
			*/
			u32 GetSubMeshCount( ) const;

			/*
			* @brief
			*/
			const VertexDataDeclaration& GetVertexDeclaration( );

		protected:

			/*
			* @brief
			*/
			void SetVertexDecl( const VertexDataDeclaration& decl );

			/*
			* @brief
			*/
			Result Release( ); 

			/*
			* @brief
			*/
			u32 GetBaseVertexID( const u32& subMeshID );

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

			/*
			* @brief
			*/
			SubMesh* ConstructSubmesh( );

		protected: 
			VertexDataDeclaration mVertexDecl;
			Vector< SubMesh* > mSubMeshes;
	}; 
}


#endif