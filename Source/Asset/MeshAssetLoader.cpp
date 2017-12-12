// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.cpp

#include "Asset/MeshAssetLoader.h" 

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

		mesh->mVerticies.push_back( TL );
		mesh->mVerticies.push_back( TR );
		mesh->mVerticies.push_back( BR );
		mesh->mVerticies.push_back( BR );
		mesh->mVerticies.push_back( BL );
		mesh->mVerticies.push_back( TL );

		// Create and upload mesh data
		glGenBuffers( 1, &mesh->mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, mesh->mVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vert ) * mesh->mVerticies.size( ), &mesh->mVerticies[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &mesh->mVAO );
		glBindVertexArray( mesh->mVAO );

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
		mesh->mDrawType = GL_TRIANGLES;
		// Set draw count
		mesh->mDrawCount = mesh->mVerticies.size( ); 
		// Set mesh name
		mesh->mName = "DefaultMesh";

		// Set default
		mDefaultAsset = mesh; 
	}

	Asset* MeshAssetLoader::LoadResourceFromFile(const String& filePath )
	{
		// Construct new mesh from filepath
		Mesh* mesh = new Enjon::Mesh( filePath ); 

		return mesh; 
	}
} 

