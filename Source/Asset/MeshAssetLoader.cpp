// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.cpp

#include "Asset/MeshAssetLoader.h" 

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
	
	void MeshAssetLoader::RegisterDefaultAsset( )
	{
		Mesh* mesh = new Enjon::Mesh;

		// Shared normal
		Enjon::Vec3 N( 0.0f, 0.0f, 1.0f );
		Enjon::Vec3 T( 0.0f, 1.0f, 0.0f );

		CREATE_QUAD_VERTEX( TL, 0.0f, 0.0f, 0.0f, 0.0f )
		CREATE_QUAD_VERTEX( TR, 1.0f, 0.0f, 1.0f, 0.0f )
		CREATE_QUAD_VERTEX( BR, 1.0f, 1.0f, 1.0f, 1.0f )
		CREATE_QUAD_VERTEX( BL, 0.0f, 1.0f, 0.0f, 1.0f )

		SubMesh sm; 

		sm.mVerticies.push_back( TL );
		sm.mVerticies.push_back( TR );
		sm.mVerticies.push_back( BR );
		sm.mVerticies.push_back( BR );
		sm.mVerticies.push_back( BL );
		sm.mVerticies.push_back( TL );

		// Create and upload mesh data
		glGenBuffers( 1, &sm.mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, sm.mVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vert ) * sm.mVerticies.size( ), &sm.mVerticies[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &sm.mVAO );
		glBindVertexArray( sm.mVAO );

		// Position
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Position ) );
		// Normal
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Normals ) );
		// Tangent
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Tangent ) );
		// UV
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, UV ) );

		// Unbind VAO
		glBindVertexArray( 0 );

		// Set draw type
		sm.mDrawType = GL_TRIANGLES;
		// Set draw count
		sm.mDrawCount = sm.mVerticies.size( ); 
		// Set mesh name
		mesh->mName = "DefaultMesh";

		// Add submesh to meshs
		mesh->mSubMeshes.push_back( sm );

		// Set default
		mDefaultAsset = mesh; 
	}

	//=====================================================================================================

	Asset* MeshAssetLoader::LoadResourceFromFile(const String& filePath )
	{
		// Construct new mesh from filepath
		//Mesh* mesh = new Enjon::Mesh( filePath ); 

		// Assimp crap
		Assimp::Importer importer;
		//const aiScene* scene = importer.ReadFile( filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals );
		const aiScene* scene = importer.ReadFile( filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals );

		if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
		{
			// Error 
		} 

		// Construct new mesh to be filled out
		Mesh* mesh = new Mesh( );

		// Process node of mesh
		ProcessNode( scene->mRootNode, scene, mesh );

		// Return mesh
		return mesh; 
	} 

	//=====================================================================================================

	void MeshAssetLoader::ProcessNode( aiNode* node, const aiScene* scene, Mesh* mesh )
	{ 
		// Process all meshes in node
		for ( u32 i = 0; i < node->mNumMeshes; ++i  ) 
		{
			aiMesh* aim = scene->mMeshes[ node->mMeshes[ i ] ]; 

			// Construct submesh from aiMesh and scene
			SubMesh sm = ProcessMesh( aim, scene ); 

			// Add submesh to mesh
			mesh->mSubMeshes.push_back( sm );
		}

		// Process all children in node
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			ProcessNode( node->mChildren[ i ], scene, mesh );
		}
	}

	//=====================================================================================================

	SubMesh MeshAssetLoader::ProcessMesh( aiMesh* mesh, const aiScene* scene )
	{
		// SubMesh to construct and fill out
		SubMesh sm; 

		// For each vertex in mesh
		for ( u32 i = 0; i < mesh->mNumVertices; ++i )
		{
			Vert vertex = { };

			// Vertex position
			vertex.Position[ 0 ] = mesh->mVertices[ i ].x;
			vertex.Position[ 1 ] = mesh->mVertices[ i ].y;
			vertex.Position[ 2 ] = mesh->mVertices[ i ].z;
			// Vertex normal
			if ( mesh->mNormals )
			{
				vertex.Normals[ 0 ] = mesh->mNormals[ i ].x;
				vertex.Normals[ 1 ] = mesh->mNormals[ i ].y;
				vertex.Normals[ 2 ] = mesh->mNormals[ i ].z; 
			}
			else
			{
				vertex.Normals[ 0 ] = 0.0f;
				vertex.Normals[ 1 ] = 0.0f;
				vertex.Normals[ 2 ] = 1.0f;
			}
			// Vertex uvs 
			if ( mesh->mTextureCoords[ 0 ] )
			{
				vertex.UV[ 0 ] = mesh->mTextureCoords[ 0 ][ i ].x; 
				vertex.UV[ 1 ] = mesh->mTextureCoords[ 0 ][ i ].y; 
			}
			else
			{
				vertex.UV[ 0 ] = 0.0f;
				vertex.UV[ 1 ] = 0.0f;
			}

			// Push back vertex into submesh
			sm.mVerticies.push_back( vertex ); 
		}

		// Compute tangents
		for ( int i = 0; i < sm.mVerticies.size( ); i += 3 )
		{
			auto& Vert1 = sm.mVerticies.at( i );
			auto& Vert2 = sm.mVerticies.at( i + 1 );
			auto& Vert3 = sm.mVerticies.at( i + 2 );

			Vec3 pos1 = Vec3( Vert1.Position[ 0 ], Vert1.Position[ 1 ], Vert1.Position[ 2 ] );
			Vec3 pos2 = Vec3( Vert2.Position[ 0 ], Vert2.Position[ 1 ], Vert2.Position[ 2 ] );
			Vec3 pos3 = Vec3( Vert3.Position[ 0 ], Vert3.Position[ 1 ], Vert3.Position[ 2 ] );

			Vec2 uv1 = Vec2( Vert1.UV[ 0 ], Vert1.UV[ 1 ] );
			Vec2 uv2 = Vec2( Vert2.UV[ 0 ], Vert2.UV[ 1 ] );
			Vec2 uv3 = Vec2( Vert3.UV[ 0 ], Vert3.UV[ 1 ] );

			// calculate tangent vectors of both triangles
			Vec3 tangent;

			// - triangle 1
			Vec3 edge1 = pos2 - pos1;
			Vec3 edge2 = pos3 - pos1;
			Enjon::Vec2 deltaUV1 = uv2 - uv1;
			Enjon::Vec2 deltaUV2 = uv3 - uv1;

			f32 f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );

			tangent.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
			tangent.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
			tangent.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
			tangent = Vec3::Normalize( tangent );

			// Set tangents for the verticies
			Vert1.Tangent[ 0 ] = tangent.x;
			Vert1.Tangent[ 1 ] = tangent.y;
			Vert1.Tangent[ 2 ] = tangent.z;

			Vert2.Tangent[ 0 ] = tangent.x;
			Vert2.Tangent[ 1 ] = tangent.y;
			Vert2.Tangent[ 2 ] = tangent.z;

			Vert3.Tangent[ 0 ] = tangent.x;
			Vert3.Tangent[ 1 ] = tangent.y;
			Vert3.Tangent[ 2 ] = tangent.z;
		}

		// Create and upload mesh data
		glGenBuffers( 1, &sm.mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, sm.mVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vert ) * sm.mVerticies.size( ), &sm.mVerticies[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &sm.mVAO );
		glBindVertexArray( sm.mVAO );

		// Position
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Position ) );
		// Normal
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Normals ) );
		// Tangent
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Tangent ) );
		// UV
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, UV ) );

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
} 

