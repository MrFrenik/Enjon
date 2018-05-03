// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.cpp

#include "Asset/MeshAssetLoader.h" 
#include "Asset/AssetManager.h"
#include "Graphics/Skeleton.h"
#include "Graphics/SkeletalAnimation.h"
#include "ImGui/ImGuiManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Enjon
{
	//==========================================================================================

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

	//==========================================================================================

	MeshAssetLoader::MeshAssetLoader()
	{
	} 

	//==========================================================================================

	MeshAssetLoader::~MeshAssetLoader()
	{
	}

	//==========================================================================================

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

	//=====================================================================================================

	bool MeshAssetLoader::HasMesh( aiNode* node, const aiScene* scene )
	{ 
		// Process all meshes in node
		bool hasMesh = false;
		if ( node->mNumMeshes != 0 )
		{
			return true;
		}

		// Process all children in node to determine if they contain any animation data as well
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			hasMesh |= HasMesh( node->mChildren[i], scene );
		}

		return hasMesh;
	}

	//=====================================================================================================

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

		// Detect whether or not scene has any mesh data
		bool hasMesh = HasMesh( scene->mRootNode, scene );

		if ( hasMesh )
		{
			// Mesh to construct
			Mesh* mesh = new Mesh( ); 

			// If has skeleton, then construct skeleton from here
			if ( hasSkeleton )
			{ 
				// Construct decl for new mesh
				VertexDataDeclaration decl;
				decl.Add( VertexAttributeFormat::Float3 );			// Position
				decl.Add( VertexAttributeFormat::Float3 );			// Normal
				decl.Add( VertexAttributeFormat::Float3 );			// Tangent
				decl.Add( VertexAttributeFormat::Float2 );			// UV
				decl.Add( VertexAttributeFormat::Float4 );			// BoneIndices
				decl.Add( VertexAttributeFormat::Float4 );			// BoneWeights

				// Set vertex decl for mesh
				mesh->SetVertexDecl( decl );

				// Construct new skeleton
				Skeleton* skeleton = new Skeleton( );

				// Store scene's global inverse transform in skeleton
				skeleton->mGlobalInverseTransform = AIMat4x4ToMat4x4( scene->mRootNode->mTransformation.Inverse( ) ); 

				// Calculate bone weight size and resize vector
				u32 totalVertCount = 0;
				for ( u32 i = 0; i < scene->mNumMeshes; ++i )
				{
					totalVertCount += scene->mMeshes[i]->mNumVertices;
				}
				skeleton->mVertexJointData.resize( totalVertCount * ENJON_MAX_NUM_JOINTS_PER_VERTEX );

				// Continue
				ProcessNodeSkeletal( scene->mRootNode, scene, skeleton, mesh ); 

				// Build the bone heirarchy for this skeleton
				BuildBoneHeirarchy( scene->mRootNode, nullptr, skeleton );

				// Set root bone id of skeleton
				skeleton->mRootID = skeleton->mJoints.empty() ? 0 : skeleton->mJoints.at( 0 ).mID; 

				// Store the skeleton for now ( totally just for debugging )
				mSkeletons.push_back( skeleton );
			} 
			// Non skeletal-mesh
			else
			{
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
			} 

			// Return mesh
			return mesh; 
		}
		// Gon' build that animation, bro
		else if ( hasAnimation )
		{ 
			// Need to have a skeleton to refer to, so for now if there isn't one, we're going to bail
			if ( mSkeletons.empty( ) )
			{
				return nullptr;
			}

			// Grab the first skeleton
			Skeleton* skeleton = mSkeletons.at( 0 );

			// Now you may proceed...
			SkeletalAnimation* animation = new SkeletalAnimation( ); 

			// Process and fill out the skeletal animation
			ProcessAnimNode( scene->mRootNode, scene, skeleton, animation ); 

			// Push back animation
			mAnimations.push_back( animation ); 
		}

		// No mesh created
		return nullptr; 
	} 

	//===================================================================================================== 

	void MeshAssetLoader::ProcessAnimNode( aiNode* node, const aiScene* scene, Skeleton* skeleton, SkeletalAnimation* animation )
	{ 
		if ( !skeleton )
		{
			// Error...
			return;
		}

		aiAnimation* aiAnim = scene->mAnimations[ 0 ];

		// Resize channel data for animation
		animation->mChannelData.resize( skeleton->mJoints.size() );

		// Set ticks per second for the animation
		animation->mTicksPerSecond = aiAnim->mTicksPerSecond;

		// Set duration in ticks for animation
		animation->mNumberOfTicks = aiAnim->mDuration;

		Vector< aiNodeAnim* > aiAnimNodes;

		for ( u32 i = 0; i < aiAnim->mNumChannels; ++i )
		{
			aiNodeAnim* animNode = aiAnim->mChannels[ i ];
			String channelName = animNode->mNodeName.C_Str();

			if ( !skeleton->HasJoint( channelName ) )
			{
				// Error...
				std::cout << "Ain't got that joint: " + channelName + "\n";
				continue;
				//return;
			}

			// Get index of joint
			u32 jointID = skeleton->mJointNameLookup[ channelName ];

			// Get channel by id
			ChannelData* channelData = &animation->mChannelData.at( jointID );

			// Position data
			for ( u32 i = 0; i < animNode->mNumPositionKeys; ++i )
			{ 
				auto pos = animNode->mPositionKeys[ i ];
				channelData->mPositionKeys.emplace_back( (f32)pos.mTime, Vec3( pos.mValue.x, pos.mValue.y, pos.mValue.z ) ); 
			}

			// Rotation data
			for ( u32 i = 0; i < animNode->mNumRotationKeys; ++i )
			{ 
				auto rot = animNode->mRotationKeys[ i ];
				channelData->mRotationKeys.emplace_back( (f32)rot.mTime, Quaternion( rot.mValue.x, rot.mValue.y, rot.mValue.z, rot.mValue.w ) ); 
			}

			// Scale data
			for ( u32 i = 0; i < animNode->mNumScalingKeys; ++i )
			{ 
				auto scl = animNode->mScalingKeys[ i ];
				channelData->mScaleKeys.emplace_back( (f32)scl.mTime, Vec3( scl.mValue.x, scl.mValue.y, scl.mValue.z ) ); 
			} 
		} 
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

	void PrintHeirarchy( const aiNode* node, u32 indent )
	{
		// Indentation
		for ( u32 i = 0; i < indent; ++i )
		{
			std::cout << " ";
		}
		std::cout << "* ";
		// Print node name
		std::cout << node->mName.C_Str() << "\n";

		// Print all children
		for ( u32 i = 0; i < node->mNumChildren; ++i )
		{
			PrintHeirarchy( node->mChildren[ i ], indent + 1 );
		}
	}

	void MeshAssetLoader::BuildBoneHeirarchy( const aiNode* node, const aiNode* parent, Skeleton* skeleton )
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
					if ( skeleton->mVertexJointData.at( vertID ).mWeights[k] == 0.0f )
					{
						skeleton->mVertexJointData.at( vertID ).mWeights[k] = weight;
						skeleton->mVertexJointData.at( vertID ).mIDS[k] = jointID;

						// We set a weight, so break out
						break;
					}
				} 
			} 
		} 
		

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

			// Get absolute vertex id from relative id
			u32 vertID = baseVertexID + i; 

			// Bone Indices
			for ( u32 i = 0; i < ENJON_MAX_NUM_JOINTS_PER_VERTEX; ++i )
			{
				sm->mVertexData.Write< f32 >( (f32)skeleton->mVertexJointData.at( vertID ).mIDS[ i ] );
			}

			// Bone Weights
			for ( u32 i = 0; i < ENJON_MAX_NUM_JOINTS_PER_VERTEX; ++i ) 
			{
				sm->mVertexData.Write< f32 >( skeleton->mVertexJointData.at( vertID ).mWeights[ i ] );
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

	void MeshAssetLoader::BeginImporting( const String& filepath )
	{
		// Do things here...
	}

	//=====================================================================================================

	const ImportOptions* MeshAssetLoader::GetImportOptions( ) const
	{
		return &mImportOptions;
	} 

	//=====================================================================================================

	Result MeshImportOptions::OnEditorView( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		//  Create skeleton
		if ( mShowSkeletonCreateDialogue )
		{
			bool createSkeleton = mCreateSkeleton;
			if ( igm->CheckBox( "Skeleton", &createSkeleton ) )
			{
				mCreateSkeleton = createSkeleton; 
			}
		}

		if ( mShowMeshCreateDialogue )
		{ 
			bool createMesh = mCreateMesh;
			if ( igm->CheckBox( "Mesh", &createMesh ) )
			{
				mCreateMesh = createMesh;
			}
		} 

		if ( mShowAnimationCreateDialogue )
		{
			bool createAnimations = mCreateAnimations;
			if ( igm->CheckBox( "Animation", &createAnimations ) )
			{
				mCreateAnimations = createAnimations;
			}
		}

		// Skeletal asset drop down
		if ( mShowAnimationCreateDialogue && !mShowSkeletonCreateDialogue )   // However this would work...
		{
			// Grab all skeletons in database
			const HashMap< String, AssetRecordInfo >* skeletons = EngineSubsystem( AssetManager )->GetAssets< Skeleton >();	

			// Need combo box...	
			for ( auto& s : *skeletons )
			{
				// Assign skeleton
				if ( igm->Selectable( s.second.GetAssetName() ) )
				{
					mSkeletonAsset = s.second.GetAsset();	
				}
			}
		}

		// Make sure can import at this point
		if ( igm->Button( "Import" ) )
		{
			// Load asset into database
			EngineSubsystem( AssetManager )->AddToDatabase( this );

			// Return successful operation
			return Result::SUCCESS;
		}

		if ( igm->Button( "Cancel" ) )
		{
			return Result::SUCCESS;
		}

		return Result::PROCESS_RUNNING;
	} 
	
	//=====================================================================================================
} 

