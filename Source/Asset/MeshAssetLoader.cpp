// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.cpp

#include "Asset/MeshAssetLoader.h" 
#include "Graphics/Skeleton.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Enjon
{
	MeshAssetLoader::MeshAssetLoader()
	{
	} 

	MeshAssetLoader::~MeshAssetLoader()
	{
	}

#define CREATE_QUAD_VERTEX( VertexName, X, Y, U, V )\
	Vert VertexName = { };\
	VertexName.Position[ 0 ] = X;\
	VertexName.Position[ 1 ] = Y;\
	VertexName.Position[ 2 ] = 0.0f;\
	VertexName.Normals[ 0 ] = N.x;\
	VertexName.Normals[ 1 ] = N.y;\
	VertexName.Normals[ 2 ] = N.z;\
	VertexName.UV[ 0 ] = U;\
	VertexName.UV[ 1 ] = V;\
	VertexName.Tangent[ 0 ] = T.x;\
	VertexName.Tangent[ 1 ] = T.y;\
	VertexName.Tangent[ 2 ] = T.z;

#define WRITE_VERT_DATA( vertData, buffer )\
	buffer.Write< f32 >( vertData.Position[ 0 ] );\
	buffer.Write< f32 >( vertData.Position[ 1 ] );\
	buffer.Write< f32 >( vertData.Position[ 2 ] );\
	buffer.Write< f32 >( vertData.Normals[ 0 ] );\
	buffer.Write< f32 >( vertData.Normals[ 1 ] );\
	buffer.Write< f32 >( vertData.Normals[ 2 ] );\
	buffer.Write< f32 >( vertData.Tangent[ 0 ] );\
	buffer.Write< f32 >( vertData.Tangent[ 1 ] );\
	buffer.Write< f32 >( vertData.Tangent[ 2 ] );\
	buffer.Write< f32 >( vertData.UV[ 0 ] );\
	buffer.Write< f32 >( vertData.UV[ 1 ] );
	
	void MeshAssetLoader::RegisterDefaultAsset( )
	{
		Mesh* mesh = new Enjon::Mesh;

		// Construct new vertex decl
		VertexDataDeclaration decl;
		decl.Add( VertexAttributeFormat::Float3 );			// Position
		decl.Add( VertexAttributeFormat::Float3 );			// Normal
		decl.Add( VertexAttributeFormat::Float3 );			// Tangent
		decl.Add( VertexAttributeFormat::Float2 );			// UV

		// Set vertex declaration
		mesh->SetVertexDecl( decl );

		// Shared normal
		Enjon::Vec3 N( 0.0f, 0.0f, 1.0f );
		Enjon::Vec3 T( 0.0f, 1.0f, 0.0f );

		CREATE_QUAD_VERTEX( TL, 0.0f, 0.0f, 0.0f, 0.0f )
		CREATE_QUAD_VERTEX( TR, 1.0f, 0.0f, 1.0f, 0.0f )
		CREATE_QUAD_VERTEX( BR, 1.0f, 1.0f, 1.0f, 1.0f )
		CREATE_QUAD_VERTEX( BL, 0.0f, 1.0f, 0.0f, 1.0f )

		// Construct new submesh
		SubMesh* sm = mesh->ConstructSubmesh( );

		WRITE_VERT_DATA( TL, sm->mVertexData )
		WRITE_VERT_DATA( BL, sm->mVertexData ) 
		WRITE_VERT_DATA( BR, sm->mVertexData )
		WRITE_VERT_DATA( BR, sm->mVertexData )
		WRITE_VERT_DATA( TR, sm->mVertexData )
		WRITE_VERT_DATA( TL, sm->mVertexData )

		// Create and upload mesh data
		glGenBuffers( 1, &sm->mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, sm->mVBO );
		glBufferData( GL_ARRAY_BUFFER, sm->mVertexData.GetSize( ), sm->mVertexData.GetData( ), GL_STATIC_DRAW );
		glGenVertexArrays( 1, &sm->mVAO );
		glBindVertexArray( sm->mVAO );

		// Get vertex data decl from owning mesh
		const VertexDataDeclaration& vertDecl = mesh->GetVertexDeclaration( );

		// Grab total size in bytes for data declaration
		usize vertexDeclSize = vertDecl.GetSizeInBytes( );

		// Vertex Attributes
		for ( u32 i = 0; i < vertDecl.mDecl.size(); ++i )
		{
			// Grab attribute
			VertexAttributeFormat attribute = vertDecl.mDecl.at( i );

			// Enable vertex attribute array
			glEnableVertexAttribArray( i );

			// Upload attribute
			switch ( attribute )
			{
				case VertexAttributeFormat::Float4:
				{
					glVertexAttribPointer( i, 4, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float3:
				{
					glVertexAttribPointer( i, 3, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float2:
				{
					glVertexAttribPointer( i, 2, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float:
				{
					glVertexAttribPointer( i, 1, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt4:
				{
					glVertexAttribPointer( i, 4, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt3:
				{
					glVertexAttribPointer( i, 3, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt2:
				{
					glVertexAttribPointer( i, 2, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt:
				{
					glVertexAttribPointer( i, 1, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;
			}
		} 

		// Unbind VAO
		glBindVertexArray( 0 );

		// Set draw type
		sm->mDrawType = GL_TRIANGLES;
		// Set draw count
		sm->mDrawCount = sm->mVertexData.GetSize( ) / vertDecl.GetSizeInBytes( ); 
		// Set mesh name
		mesh->mName = "DefaultMesh"; 

		// Set default
		mDefaultAsset = mesh; 
	}

	//=====================================================================================================

	bool MeshAssetLoader::HasSkeleton( aiNode* node, const aiScene* scene )
	{
		// Process all meshes in node
		bool hasBones = false;
		for ( u32 i = 0; i < node->mNumMeshes; ++i )
		{
			aiMesh* aim = scene->mMeshes[node->mMeshes[i]];

			if ( aim->HasBones( ) )
			{
				return true;
			}
		}

		// Process all children in node to determine if they contain any animation data as well
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			hasBones |= HasSkeleton( node->mChildren[i], scene );
		}

		return hasBones;
	}

	Asset* MeshAssetLoader::LoadResourceFromFile(const String& filePath )
	{
		// Construct new mesh from filepath 
		Assimp::Importer importer;

		// NOTE(): Flipping UVs FUCKS IT ALL because I'm already flipping UVs in the shader generation process (shadergraph). Need to fix this.  
		const aiScene* scene = importer.ReadFile( filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace );

		if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
		{
			// Error 
		} 

		// Check whether or not the scene has animations
		bool hasAnimation = scene->HasAnimations( );

		// Will construct the skeleton here if has bones and we want to create a new skeleton in the import process
		bool hasSkeleton = HasSkeleton( scene->mRootNode, scene );

		// If has skeleton, then construct skeleton from here
		//if ( hasSkeleton )
		//{
		//	SkeletalMesh* mesh = new SkeletalMesh( );
		//	ProcessNodeSkeletal( scene->mRootNode, scene, mesh );
		//}
 
		// Construct new mesh to be filled out
		Mesh* mesh = new Mesh( ); 

		// Construct decl for new mesh
		VertexDataDeclaration decl;
		decl.Add( VertexAttributeFormat::Float3 );			// Position
		decl.Add( VertexAttributeFormat::Float3 );			// Normal
		decl.Add( VertexAttributeFormat::Float3 );			// Tangent
		decl.Add( VertexAttributeFormat::Float2 );			// UV

		// Set vertex decl for mesh
		mesh->SetVertexDecl( decl );

		// Process node of mesh
		ProcessNode( scene->mRootNode, scene, mesh );

		// Return mesh
		return mesh; 
	} 

	//===================================================================================================== 

	void MeshAssetLoader::ProcessNodeSkeletal( aiNode* node, const aiScene* scene, SkeletalMesh* mesh )
	{
		// Process all meshes in node
		for ( u32 i = 0; i < node->mNumMeshes; ++i  ) 
		{
			aiMesh* aim = scene->mMeshes[ node->mMeshes[ i ] ]; 

			// Construct submesh from aiMesh and scene
			SubMesh sm = ProcessSkeletalMesh( aim, scene ); 
		}

		// Process all children in node
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			ProcessNode( node->mChildren[ i ], scene, mesh );
		} 
	}

	//===================================================================================================== 

	SubMesh MeshAssetLoader::ProcessSkeletalMesh( aiMesh* mesh, const aiScene* scene )
	{
		//struct SkeletalVertexData
		//{ 
		//	Vec3	mPosition;
		//	Vec3	mNormal;
		//	Vec3	mTangent;
		//	Vec2	mUV;
		//	u32		mJointIndicies[ENJON_MAX_NUM_JOINTS_PER_VERTEX];
		//	f32		mJointWeights[ENJON_MAX_NUM_JOINTS_PER_VERTEX];
		//};

		// SubMesh to construct and fill out
		SkeletalSubMesh sm; 

		// For each vertex in mesh
		for ( u32 i = 0; i < mesh->mNumVertices; ++i ) 
		{ 
			SkeletalVertexData vertex = { }; 

			// Vertex position
			vertex.mPosition.x = mesh->mVertices[ i ].x;
			vertex.mPosition.y = mesh->mVertices[ i ].y;
			vertex.mPosition.z = mesh->mVertices[ i ].z;
			// Vertex normal
			if ( mesh->mNormals )
			{
				vertex.mNormal.x = mesh->mNormals[ i ].x;
				vertex.mNormal.y = mesh->mNormals[ i ].y;
				vertex.mNormal.z = mesh->mNormals[ i ].z; 
			}
			else
			{
				vertex.mNormal = Vec3( 0.0f );
			}
			// Vertex uvs 
			if ( mesh->mTextureCoords[ 0 ] )
			{
				vertex.mUV.x = mesh->mTextureCoords[ 0 ][ i ].x; 
				vertex.mUV.y = mesh->mTextureCoords[ 0 ][ i ].y; 
			}
			else
			{
				vertex.mUV = Vec2( 0.0f );
			}

			if ( mesh->mTangents )
			{
				vertex.mTangent.x = mesh->mTangents[ i ].x;
				vertex.mTangent.y = mesh->mTangents[ i ].y;
				vertex.mTangent.z = mesh->mTangents[ i ].z;
			}

			// Push back vertex into submesh
			sm.mSkeletalVertexData.push_back( vertex ); 
		} 

		// Create and upload mesh data
		glGenBuffers( 1, &sm.mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, sm.mVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( SkeletalVertexData ) * sm.mSkeletalVertexData.size( ), &sm.mSkeletalVertexData[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &sm.mVAO );
		glBindVertexArray( sm.mVAO );

		// Position
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( SkeletalVertexData ), ( void* )offsetof( SkeletalVertexData, mPosition) );
		// Normal
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( SkeletalVertexData ), ( void* )offsetof( SkeletalVertexData, mNormal ) );
		// Tangent
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( SkeletalVertexData ), ( void* )offsetof( SkeletalVertexData, mTangent ) );
		// UV
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( SkeletalVertexData ), ( void* )offsetof( SkeletalVertexData, mUV ) );
		// Joint Indices
		glEnableVertexAttribArray( 4 );
		glVertexAttribPointer( 4, 4, GL_UNSIGNED_INT, GL_FALSE, sizeof( SkeletalVertexData ), (void*)offsetof( SkeletalVertexData, mJointIndices ) );
		// Joint Weights
		glEnableVertexAttribArray( 5 );
		glVertexAttribPointer( 5, 4, GL_FLOAT, GL_FALSE, sizeof( SkeletalVertexData ), (void*)offsetof( SkeletalVertexData, mJointWeights ) );

		// Unbind mVAO
		glBindVertexArray( 0 );

		// Set draw type
		sm.mDrawType = GL_TRIANGLES;
		// Set draw count
		sm.mDrawCount = sm.mVerticies.size( ); 

		// Return submesh
		return sm;
	}

	//=====================================================================================================

	void MeshAssetLoader::ProcessNode( aiNode* node, const aiScene* scene, Mesh* mesh )
	{ 
		// Process all meshes in node
		for ( u32 i = 0; i < node->mNumMeshes; ++i  ) 
		{
			aiMesh* aim = scene->mMeshes[ node->mMeshes[ i ] ]; 

			// Add submesh to mesh
			ProcessMesh( aim, scene, mesh );
		}

		// Process all children in node
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			ProcessNode( node->mChildren[ i ], scene, mesh );
		}
	}

	//=====================================================================================================

	void MeshAssetLoader::ProcessMesh( aiMesh* mesh, const aiScene* scene, Mesh* owningMesh )
	{ 
		// Construct new mesh in owning mesh and get pointer to it
		SubMesh* sm = owningMesh->ConstructSubmesh( );

		// Get decl from mesh
		const VertexDataDeclaration& vertDecl = owningMesh->GetVertexDeclaration( ); 

		// Load vertex data into submesh vertex buffer 
		for ( u32 i = 0; i < mesh->mNumVertices; ++i )
		{ 
			// Position
			sm->mVertexData.Write< f32 >( mesh->mVertices[i].x );
			sm->mVertexData.Write< f32 >( mesh->mVertices[i].y );
			sm->mVertexData.Write< f32 >( mesh->mVertices[i].z );

			// Normal
			if ( mesh->mNormals )
			{
				sm->mVertexData.Write< f32 >( mesh->mNormals[i].x );
				sm->mVertexData.Write< f32 >( mesh->mNormals[i].y );
				sm->mVertexData.Write< f32 >( mesh->mNormals[i].z );
			}
			else
			{
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 1.0f ); 
			} 

			// Tangent
			if ( mesh->mTangents )
			{
				sm->mVertexData.Write< f32 >( mesh->mTangents[i].x );
				sm->mVertexData.Write< f32 >( mesh->mTangents[i].y );
				sm->mVertexData.Write< f32 >( mesh->mTangents[i].z ); 
			}
			else
			{
				sm->mVertexData.Write< f32 >( 1.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
			}

			// UV
			if ( mesh->mTextureCoords[ 0 ] )
			{
				sm->mVertexData.Write< f32 >( mesh->mTextureCoords[0][i].x );
				sm->mVertexData.Write< f32 >( mesh->mTextureCoords[0][i].y );
			}
			else
			{
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
			}
		}

		// Create and upload mesh data 
		glGenBuffers( 1, &sm->mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, sm->mVBO );
		glBufferData( GL_ARRAY_BUFFER, sm->mVertexData.GetSize( ), sm->mVertexData.GetData( ), GL_STATIC_DRAW ); 
		glGenVertexArrays( 1, &sm->mVAO );
		glBindVertexArray( sm->mVAO );

		// Grab total size in bytes for data declaration
		usize vertexDeclSize = vertDecl.GetSizeInBytes( );

		// Vertex Attributes
		for ( u32 i = 0; i < vertDecl.mDecl.size(); ++i )
		{
			// Grab attribute
			VertexAttributeFormat attribute = vertDecl.mDecl.at( i );

			// Enable vertex attribute array
			glEnableVertexAttribArray( i );

			// Upload attribute
			switch ( attribute )
			{
				case VertexAttributeFormat::Float4:
				{
					glVertexAttribPointer( i, 4, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float3:
				{
					glVertexAttribPointer( i, 3, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float2:
				{
					glVertexAttribPointer( i, 2, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float:
				{
					glVertexAttribPointer( i, 1, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt4:
				{
					glVertexAttribPointer( i, 4, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt3:
				{
					glVertexAttribPointer( i, 3, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt2:
				{
					glVertexAttribPointer( i, 2, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt:
				{
					glVertexAttribPointer( i, 1, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;
			}
		} 

		// Unbind vertex array
		glBindVertexArray( 0 );

		// Set draw type
		sm->mDrawType = GL_TRIANGLES;

		// Set draw count
		sm->mDrawCount = sm->mVertexData.GetSize( ) / vertDecl.GetSizeInBytes( ); 
	}

	//=====================================================================================================
} 

