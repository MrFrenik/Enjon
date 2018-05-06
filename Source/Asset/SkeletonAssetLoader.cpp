// @file SkeletonAssetLoader.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Asset/MeshAssetLoader.h"
#include "Asset/SkeletonAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Utils/FileUtils.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Enjon
{ 

	//=========================================================================

	Mat4x4 AIMat4x4ToMat4x4( const aiMatrix4x4& aiMat )
	{ 
		Mat4x4 mat4 = Mat4x4::Identity( ); 

		mat4.elements[ 0 ] = aiMat.a1;
		mat4.elements[ 1 ] = aiMat.b1;
		mat4.elements[ 2 ] = aiMat.c1;
		mat4.elements[ 3 ] = aiMat.d1;

		mat4.elements[ 4 ] = aiMat.a2;
		mat4.elements[ 5 ] = aiMat.b2;
		mat4.elements[ 6 ] = aiMat.c2;
		mat4.elements[ 7 ] = aiMat.d2;

		mat4.elements[ 8 ] = aiMat.a3;
		mat4.elements[ 9 ] = aiMat.b3;
		mat4.elements[ 10 ] = aiMat.c3;
		mat4.elements[ 11 ] = aiMat.d3;

		mat4.elements[ 12 ] = aiMat.a4;
		mat4.elements[ 13 ] = aiMat.b4;
		mat4.elements[ 14 ] = aiMat.c4;
		mat4.elements[ 15 ] = aiMat.d4;

		return mat4;
	}

	//=========================================================================

	aiMatrix4x4 Mat4x4ToAIMat4x4( const Mat4x4& mat4 )
	{ 
		aiMatrix4x4 aiMat;

		aiMat.a1 = mat4.elements[ 0 ];
		aiMat.b1 = mat4.elements[ 1 ];
		aiMat.c1 = mat4.elements[ 3 ];
		aiMat.d1 = mat4.elements[ 3 ];

		aiMat.a2 = mat4.elements[ 4 ];
		aiMat.b2 = mat4.elements[ 5 ];
		aiMat.c2 = mat4.elements[ 6 ];
		aiMat.d2 = mat4.elements[ 7 ];

		aiMat.a3 = mat4.elements[ 8 ];
		aiMat.b3 = mat4.elements[ 9 ];
		aiMat.c3 = mat4.elements[ 10 ];
		aiMat.d3 = mat4.elements[ 11 ];

		aiMat.a4 = mat4.elements[ 12 ];
		aiMat.b4 = mat4.elements[ 13 ];
		aiMat.c4 = mat4.elements[ 14 ];
		aiMat.d4 = mat4.elements[ 15 ]; 

		return aiMat;
	}

	//=========================================================================

	void SkeletonAssetLoader::DecomposeMatrix( const Mat4x4& original, Vec3& position, Vec3& scale, Quaternion& rotation )
	{
		aiMatrix4x4 mat = Mat4x4ToAIMat4x4( original );
		aiVector3D aiPos, aiScl;
		aiQuaternion aiRot;
		mat.Decompose( aiScl, aiRot, aiPos );

		position = Vec3( aiPos.x, aiPos.y, aiPos.z );
		scale = Vec3( aiScl.x, aiScl.y, aiScl.z );
		rotation = Quaternion( aiRot.x, aiRot.y, aiRot.z, aiRot.w );
	}

	//=========================================================================

	SkeletonAssetLoader::SkeletonAssetLoader( )
	{ 
	}

	//=========================================================================

	SkeletonAssetLoader::~SkeletonAssetLoader( )
	{ 
	} 

	//=========================================================================

	Asset* SkeletonAssetLoader::DirectImport( const ImportOptions* options )
	{
		// Need to basically act as the asset manager here...
		Skeleton* skeleton = this->LoadResourceFromImporter( options )->ConstCast< Skeleton >( );

		if ( skeleton )
		{
			// Register and cache
			MeshImportOptions mo = *options->ConstCast< MeshImportOptions >( ); 
			mo.mLoader = this;
			EngineSubsystem( AssetManager )->AddToDatabase( skeleton, &mo );
		}

		// Return mesh after processing
		return skeleton; 
	}

	//=========================================================================

	Asset* SkeletonAssetLoader::LoadResourceFromImporter( const ImportOptions* options )
	{
		MeshImportOptions* meshOptions = options->ConstCast< MeshImportOptions >( ); 
		if ( !meshOptions )
		{
			return nullptr;
		}

		// Construct new mesh from filepath 
		Assimp::Importer importer;

		// NOTE(): Flipping UVs FUCKS IT ALL because I'm already flipping UVs in the shader generation process (shadergraph). Need to fix this.  
		const aiScene* scene = importer.ReadFile( meshOptions->GetResourceFilePath( ), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace ); 
		if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
		{
			// Error 
			return nullptr;
		} 

		// Skeleton to create
		Skeleton* skeleton = new Skeleton( );
 
		// Store scene's global inverse transform in skeleton
		skeleton->mGlobalInverseTransform = AIMat4x4ToMat4x4( scene->mRootNode->mTransformation.Inverse( ) ); 

		// Process skeleton
		ProcessNodeSkeletal( scene->mRootNode, scene, skeleton );

		// Build the bone heirarchy for this skeleton
		BuildBoneHeirarchy( scene->mRootNode, nullptr, skeleton );

		// Set root id for skeleton
		skeleton->mRootID = skeleton->mJoints.empty() ? -1 : skeleton->mJoints.at( 0 ).mID;

		// Return skeleton after processing
		return skeleton; 
	} 

	//=========================================================================

	void SkeletonAssetLoader::ProcessNodeSkeletal( aiNode* node, const aiScene* scene, Skeleton* skeleton )
	{
		// Process all meshes in node
		for ( u32 i = 0; i < node->mNumMeshes; ++i  ) 
		{
			aiMesh* aim = scene->mMeshes[ node->mMeshes[ i ] ]; 

			// Load bone data
			for ( u32 i = 0; i < aim->mNumBones; ++i )
			{
				// Grab bone pointer
				aiBone* aBone = aim->mBones[i];

				// Get joint id ( which is the amount of bones )
				u32 jointID = skeleton->mJoints.size( );
				String jointName( aBone->mName.data ); 

				// If joint not found in skeleton name lookup then construct new bone and push back
				if ( skeleton->mJointNameLookup.find( jointName ) == skeleton->mJointNameLookup.end( ) )
				{ 
					// Construct new joint
					Joint joint; 
					// Set id
					joint.mID = jointID;
					// Set bone name
					joint.mName = jointName;

					// Set up joint offset
					aiMatrix4x4 offsetMatrix = aBone->mOffsetMatrix;
					joint.mInverseBindMatrix = AIMat4x4ToMat4x4( aBone->mOffsetMatrix ); 

					// Push bone back 
					skeleton->mJoints.push_back( joint ); 

					// Set mapping between bone id and name
					skeleton->mJointNameLookup[jointName] = joint.mID; 

				} 
			} 
		}

		// Process all children in node
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			ProcessNodeSkeletal( node->mChildren[ i ], scene, skeleton );
		} 
	}

	//=========================================================================

	void SkeletonAssetLoader::RegisterDefaultAsset( )
	{
		// Just create a new skeleton... Not sure what this should do yet...
		mDefaultAsset = new Skeleton( );
	}

	//========================================================================= 

	void SkeletonAssetLoader::BuildBoneHeirarchy( const aiNode* node, const aiNode* parent, Skeleton* skeleton )
	{ 
		u32 jointIndex = 0;

		// Grab joint index
		if ( skeleton->HasJoint( node->mName.C_Str( ) ) )
		{
			jointIndex = skeleton->mJointNameLookup[ node->mName.C_Str( ) ];

			// If parent is valid ( not root )
			if ( parent )
			{
				// Set parent id
				if ( skeleton->HasJoint( parent->mName.C_Str( ) ) )
				{
					u32 parentID = skeleton->mJointNameLookup[ parent->mName.C_Str( ) ];
					skeleton->mJoints.at( jointIndex ).mParentID = parentID;
				} 
			}
			else
			{
				skeleton->mJoints.at( jointIndex ).mParentID = -1;
			}

			// Set children for this node
			for ( u32 i = 0; i < node->mNumChildren; ++i )
			{
				// Grab child
				aiNode* child = node->mChildren[ i ];

				// If exists, then set index of child
				if ( skeleton->HasJoint( child->mName.C_Str( ) ) )
				{
					skeleton->mJoints.at( jointIndex ).mChildren.push_back( skeleton->mJointNameLookup[ child->mName.C_Str( ) ] );
				} 

				// Do heiarchy for this child
				BuildBoneHeirarchy( child, node, skeleton );
			}
		}
		// Can't find joint, so continue, I suppose...
		else 
		{ 
			// Children of this node, but pass in previous parent	
			for ( u32 i = 0; i < node->mNumChildren; ++i )
			{
				aiNode* child = node->mChildren[ i ]; 

				// Try to add child to parent
				if ( skeleton->HasJoint( child->mName.C_Str( ) ) )
				{
					if ( parent && skeleton->HasJoint( parent->mName.C_Str( ) ) )
					{
						u32 childIdx = skeleton->mJointNameLookup[ child->mName.C_Str( ) ];
						u32 parentIdx = skeleton->mJointNameLookup[ parent->mName.C_Str( ) ];
						skeleton->mJoints.at( parentIdx ).mChildren.push_back( childIdx );
					}
				}

				BuildBoneHeirarchy( child, parent, skeleton );
			}
		}

	}

	//=======================================================================================

	String SkeletonAssetLoader::GetAssetFileExtension( ) const
	{
		return ".eskl";
	}

	//=======================================================================================
}
