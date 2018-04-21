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
		if ( hasSkeleton )
		{
			// Construct new mesh
			Mesh* skeletalMesh = new Mesh( );
			// Construct new skeleton
			Skeleton* skeleton = new Skeleton( );

			// Calculate bone weight size and resize vector
			u32 totalVertCount = 0;
			for ( u32 i = 0; i < scene->mNumMeshes; ++i )
			{
				totalVertCount += scene->mMeshes[i]->mNumVertices;
			}
			skeleton->mVertexBoneData.resize( totalVertCount * ENJON_MAX_NUM_BONES_PER_VERTEX );

			// Continue
			ProcessNodeSkeletal( scene->mRootNode, scene, skeleton, skeletalMesh );

			// For now, just delete those two until they start being used...
			delete ( skeleton );
			delete ( skeletalMesh );

			skeleton = nullptr;
			skeletalMesh = nullptr;
		} 
 
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

	void MeshAssetLoader::ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleton, Mesh* mesh )
	{
		// Process all meshes in node
		for ( u32 i = 0; i < node->mNumMeshes; ++i  ) 
		{
			aiMesh* aim = scene->mMeshes[ node->mMeshes[ i ] ]; 

			// Construct submesh from aiMesh and scene
			ProcessSkeletalMesh( aim, scene, skeleton, mesh ); 
		}

		// Process all children in node
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			ProcessNodeSkeletal( node->mChildren[ i ], scene, skeleton, mesh );
		} 
	}

	//===================================================================================================== 

	void MeshAssetLoader::ProcessSkeletalMesh( aiMesh* aim, const aiScene* scene, Skeleton* skeleton, Mesh* mesh )
	{ 
		// Id of this submesh is the size of the number of sub-meshes in owning mesh
		u32 meshID = mesh->mSubMeshes.size( );
		u32 baseVertexID = mesh->GetBaseVertexID( meshID );

		// Construct new submesh
		SubMesh* sm = mesh->ConstructSubmesh( );

		// Load bone data
		for ( u32 i = 0; i < aim->mNumBones; ++i )
		{
			// Grab bone pointer
			aiBone* aBone = aim->mBones[i];

			u32 boneID = 0;
			String boneName( aBone->mName.data ); 

			// If joint not found in skeleton name lookup then construct new bone and push back
			if ( skeleton->mBoneNameLookup.find( boneName ) == skeleton->mBoneNameLookup.end( ) )
			{ 
				// Construct new bone
				Bone bone; 
				// Set id
				bone.mID = boneID;
				// Set bone name
				bone.mName = boneName;
				// Increment bone id
				boneID++;

				// Set up bone offset
				aiMatrix4x4 offsetMatrix = aBone->mOffsetMatrix;

				// Get transform from offset matrix
				aiVector3t<f32> scale;
				aiVector3t<f32> axis;
				f32 angle;
				aiVector3t<f32> position; 
				offsetMatrix.Decompose( scale, axis, angle, position );			// Decompose into elements

				// Construct matrix from transform 
				bone.mInverseBindMatrix = Mat4x4::Identity( );
				bone.mInverseBindMatrix *= Mat4x4::Translate( Vec3( position.x, position.y, position.z ) );
				bone.mInverseBindMatrix *= QuaternionToMat4x4( Quaternion::AngleAxis( angle, Vec3( axis.x, axis.y, axis.z ) ) );
				bone.mInverseBindMatrix *= Mat4x4::Scale( Vec3( scale.x, scale.y, scale.z ) ); 

				// Push bone back 
				skeleton->mBones.push_back( bone ); 
				// Set mapping between bone id and name
				skeleton->mBoneNameLookup[boneName] = boneID; 
			} 
			else
			{
				boneID = skeleton->mBoneNameLookup[ boneName ];
			} 

			// Set weights for vertices that this bone effects
			for ( u32 j = 0; j < aBone->mNumWeights; ++j )
			{
				// Grab weight structure
				aiVertexWeight aiWeight = aBone->mWeights[j];

				// Grab weight
				f32 weight = aiWeight.mWeight;

				// Calculate absolute vertex id
				u32 vertID = baseVertexID + aiWeight.mVertexId; 

				// Add vertex bone data
				for ( u32 k = 0, bool found = false; k < ENJON_MAX_NUM_BONES_PER_VERTEX, found != false; ++k )
				{
					// We loop the weights until we find one not set
					if ( skeleton->mVertexBoneData.at( vertID ).mIDS[k] == -1 )
					{
						skeleton->mVertexBoneData.at( vertID ).mWeights[k] = weight;
						skeleton->mVertexBoneData.at( vertID ).mIDS[k] = boneID;

						// We set a weight, so break out
						found = true;
					}
				} 
			} 
		}

		// Construct submesh here...
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

	void MeshAssetLoader::ProcessMesh( aiMesh* aim, const aiScene* scene, Mesh* mesh )
	{ 
		// Construct new mesh in owning mesh and get pointer to it
		SubMesh* sm = mesh->ConstructSubmesh( );

		// Get decl from mesh
		const VertexDataDeclaration& vertDecl = mesh->GetVertexDeclaration( ); 

		// Load vertex data into submesh vertex buffer 
		for ( u32 i = 0; i < aim->mNumVertices; ++i )
		{ 
			// Position
			sm->mVertexData.Write< f32 >( aim->mVertices[i].x );
			sm->mVertexData.Write< f32 >( aim->mVertices[i].y );
			sm->mVertexData.Write< f32 >( aim->mVertices[i].z );

			// Normal
			if ( aim->mNormals )
			{
				sm->mVertexData.Write< f32 >( aim->mNormals[i].x );
				sm->mVertexData.Write< f32 >( aim->mNormals[i].y );
				sm->mVertexData.Write< f32 >( aim->mNormals[i].z );
			}
			else
			{
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 1.0f ); 
			} 

			// Tangent
			if ( aim->mTangents )
			{
				sm->mVertexData.Write< f32 >( aim->mTangents[i].x );
				sm->mVertexData.Write< f32 >( aim->mTangents[i].y );
				sm->mVertexData.Write< f32 >( aim->mTangents[i].z ); 
			}
			else
			{
				sm->mVertexData.Write< f32 >( 1.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
				sm->mVertexData.Write< f32 >( 0.0f );
			}

			// UV
			if ( aim->mTextureCoords[ 0 ] )
			{
				sm->mVertexData.Write< f32 >( aim->mTextureCoords[0][i].x );
				sm->mVertexData.Write< f32 >( aim->mTextureCoords[0][i].y );
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
					glVertexAttribIPointer( i, 4, GL_UNSIGNED_INT, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt3:
				{
					glVertexAttribIPointer( i, 3, GL_UNSIGNED_INT, GL_FALSE, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt2:
				{
					glVertexAttribIPointer( i, 2, GL_UNSIGNED_INT, GL_FALSE, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt:
				{
					glVertexAttribIPointer( i, 1, GL_UNSIGNED_INT, GL_FALSE, (void*)vertDecl.GetByteOffset( i ) );
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

