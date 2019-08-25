// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.h

#include "Asset/SkeletalMeshAssetLoader.h"
#include "Asset/MeshAssetLoader.h"
#include "Graphics/SkeletalMesh.h"
#include "Graphics/Skeleton.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Enjon
{ 
	//========================================================================================

	void SkeletalMeshAssetLoader::RegisterDefaultAsset( )
	{
		// Not sure what this is supposed to look like...
		mDefaultAsset = new SkeletalMesh( );
	} 

	//======================================================================================== 

	Asset* SkeletalMeshAssetLoader::LoadResourceFromImporter( const ImportOptions* options )
	{
		const MeshImportOptions* meshOptions = static_cast<const MeshImportOptions*>( options );

		if ( !meshOptions )
		{
			return nullptr;
		}

		Assimp::Importer importer;

		// Read aiscene
		//const aiScene* scene = importer.ReadFile( meshOptions->GetResourceFilePath(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights ); 
		//const aiScene* scene = importer.ReadFile( meshOptions->GetResourceFilePath(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace ); 
		const aiScene* scene = importer.ReadFile( meshOptions->GetResourceFilePath( ), 
												aiProcess_Triangulate | 
												aiProcess_GenSmoothNormals | 
												aiProcess_CalcTangentSpace | 
												aiProcess_OptimizeMeshes | 
												//aiProcess_SplitLargeMeshes | 
												aiProcess_JoinIdenticalVertices |
												//aiProcess_FindDegenerates | 
												//aiProcess_FindInvalidData | 
												aiProcess_ImproveCacheLocality | 
												//aiProcess_SortByPType | 
												aiProcess_GenUVCoords
		); 
		if ( !scene || !scene->mRootNode )
		{
			// Error
			return nullptr;
		} 

		// Construct new mesh to fill out
		SkeletalMesh* mesh = new SkeletalMesh( ); 

		// Construct decl for new mesh
		VertexDataDeclaration decl;
		decl.Add( VertexAttributeFormat::Float3 );			// Position
		decl.Add( VertexAttributeFormat::Float3 );			// Normal
		decl.Add( VertexAttributeFormat::Float3 );			// Tangent
		decl.Add( VertexAttributeFormat::Float2 );			// UV
		decl.Add( VertexAttributeFormat::UnsignedInt4 );	// BoneIndices1
		decl.Add( VertexAttributeFormat::UnsignedInt4 );	// BoneIndices2
		decl.Add( VertexAttributeFormat::Float4 );			// BoneWeights
		decl.Add( VertexAttributeFormat::Float4 );			// BoneWeights2

		// Set vertex decl for mesh
		mesh->SetVertexDecl( decl );

		// Grab skeleton from import options to use for this mesh
		AssetHandle< Skeleton > skeleton = meshOptions->GetSkeleton(); 
		
		// If skeleton not valid, error
		if ( !skeleton )
		{
			delete( mesh );
			mesh = nullptr;
			return nullptr;
		} 

		// Joint data to use for vertices
		Vector< VertexJointData > vertexJointData;

		// Calculate bone weight size and resize vector
		u32 totalVertCount = 0;
		for ( u32 i = 0; i < scene->mNumMeshes; ++i )
		{
			totalVertCount += scene->mMeshes[i]->mNumVertices;
		}
		vertexJointData.resize( totalVertCount * ENJON_MAX_NUM_JOINTS_PER_VERTEX );

		// Finish processing the mesh
		ProcessNodeSkeletal( scene->mRootNode, scene, skeleton, mesh, &vertexJointData );

		// Return mesh
		return mesh;
	}

	//========================================================================================

	Asset* SkeletalMeshAssetLoader::DirectImport( const ImportOptions* options )
	{
		// Need to basically act as the asset manager here...
		SkeletalMesh* mesh = this->LoadResourceFromImporter( options )->ConstCast< SkeletalMesh >( );

		if ( mesh )
		{
			// Needs to cache the skeletal mesh now 
			MeshImportOptions mo = *options->ConstCast< MeshImportOptions >( ); 
			mesh->mSkeleton = mo.mSkeletonAsset;
			mo.mLoader = this; 
			EngineSubsystem( AssetManager )->AddToDatabase( mesh, &mo );
		}

		// Return mesh after processing
		return mesh;
	} 

	void SkeletalMeshAssetLoader::ProcessNodeSkeletal( aiNode* node, const aiScene* scene, const AssetHandle< Skeleton >& skeleton, SkeletalMesh* mesh, Vector< VertexJointData >* vertexJointData )
	{
		// Process all meshes in node
		for ( u32 i = 0; i < node->mNumMeshes; ++i  ) 
		{
			aiMesh* aim = scene->mMeshes[ node->mMeshes[ i ] ]; 

			// Construct submesh from aiMesh and scene
			ProcessSkeletalMesh( aim, scene, skeleton, mesh, vertexJointData ); 
		}

		// Process all children in node
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			ProcessNodeSkeletal( node->mChildren[ i ], scene, skeleton, mesh, vertexJointData );
		} 
	} 

	void SkeletalMeshAssetLoader::ProcessSkeletalMesh( aiMesh* aim, const aiScene* scene, const AssetHandle< Skeleton >& skeletonHandle, SkeletalMesh* mesh, Vector< VertexJointData >* vertexJointData )
	{ 
		// Id of this submesh is the size of the number of sub-meshes in owning mesh
		u32 meshID = mesh->mSubMeshes.size( );
		u32 baseVertexID = mesh->GetBaseVertexID( meshID ); 

		// Construct new submesh 
		SubMesh* sm = mesh->ConstructSubmesh( );

		// Get skeleton asset
		Skeleton* skeleton = skeletonHandle.Get( )->ConstCast< Skeleton >( );

		b32 allWeightsZero = false;

		// Load bone data
		for ( u32 i = 0; i < aim->mNumBones; ++i )
		{
			// Grab bone pointer
			aiBone* aBone = aim->mBones[i];

			// Get joint id ( which is the amount of bones )
			u32 jointID = skeleton->mJoints.size( );
			String jointName( aBone->mName.data ); 

			// If joint not found in skeleton name lookup then we can't continue to process this mesh
			if ( skeleton->mJointNameLookup.find( jointName ) == skeleton->mJointNameLookup.end( ) )
			{ 
				continue;
			} 
			else
			{
				jointID = skeleton->mJointNameLookup[ jointName ];
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
				for ( u32 k = 0; k < ENJON_MAX_NUM_JOINTS_PER_VERTEX; ++k )
				{
					// We loop the weights until we find one not set
					if ( vertexJointData->at( vertID ).mWeights[k] == 0.0f )
					{
						vertexJointData->at( vertID ).mWeights[k] = weight;
						vertexJointData->at( vertID ).mIDS[k] = jointID;

						// We set a weight, so break out
						break;
					}
				} 
			} 
		} 

		// Load vertex data into submesh vertex buffer 
		for ( u32 i = 0; i < aim->mNumFaces; ++i )
		{ 
			const aiFace& face = aim->mFaces[ i ];
			assert( face.mNumIndices == 3 );

			for ( u32 j = 0; j < face.mNumIndices; ++j )
			{
				u32 vi = face.mIndices[ j ];

				// Position
				sm->mVertexData.Write< f32 >( aim->mVertices[ vi ].x );
				sm->mVertexData.Write< f32 >( aim->mVertices[ vi ].y );
				sm->mVertexData.Write< f32 >( aim->mVertices[ vi ].z );

				// Normal
				if ( aim->mNormals )
				{
					sm->mVertexData.Write< f32 >( aim->mNormals[ vi ].x );
					sm->mVertexData.Write< f32 >( aim->mNormals[ vi ].y );
					sm->mVertexData.Write< f32 >( aim->mNormals[ vi ].z );
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
					sm->mVertexData.Write< f32 >( aim->mTangents[ vi ].x );
					sm->mVertexData.Write< f32 >( aim->mTangents[ vi ].y );
					sm->mVertexData.Write< f32 >( aim->mTangents[ vi ].z );
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
					sm->mVertexData.Write< f32 >( aim->mTextureCoords[0][ vi ].x );
					sm->mVertexData.Write< f32 >( aim->mTextureCoords[0][ vi ].y );
				}
				else
				{
					sm->mVertexData.Write< f32 >( 0.0f );
					sm->mVertexData.Write< f32 >( 0.0f );
				}

				// Get absolute vertex id from relative id
				u32 vertID = baseVertexID + vi; 

				// Bone Indices
				for ( u32 bi = 0; bi < ENJON_MAX_NUM_JOINTS_PER_VERTEX; ++bi )
				{
					sm->mVertexData.Write< u32 >( (u32)vertexJointData->at( vertID ).mIDS[ bi ] );
				}

				// Bone Weights
				for ( u32 wi = 0; wi < ENJON_MAX_NUM_JOINTS_PER_VERTEX; ++wi ) 
				{
					sm->mVertexData.Write< f32 >( vertexJointData->at( vertID ).mWeights[ wi ] );
				} 
			}
		}

		// Get decl from mesh
		const VertexDataDeclaration& vertDecl = mesh->GetVertexDeclaration( ); 

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
					glVertexAttribIPointer( i, 3, GL_UNSIGNED_INT, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt2:
				{
					glVertexAttribIPointer( i, 2, GL_UNSIGNED_INT, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt:
				{
					glVertexAttribIPointer( i, 1, GL_UNSIGNED_INT, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
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

	//===================================================================================

	String SkeletalMeshAssetLoader::GetAssetFileExtension( ) const
	{
		return ".eskm";
	}

	//===================================================================================
}
