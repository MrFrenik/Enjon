// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: MeshAssetLoader.cpp

#include "Asset/MeshAssetLoader.h" 
#include "Asset/SkeletonAssetLoader.h"
#include "Asset/SkeletalAnimationAssetLoader.h"
#include "Asset/SkeletalMeshAssetLoader.h"
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

	void MeshAssetLoader::ExplicitConstructor( )
	{
		// Set import options loader
		mImportOptions.mLoader = this;
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
		Mesh* mesh = new Mesh();

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

	//============================================================================================================

	Asset* MeshAssetLoader::LoadResourceFromImporter( const ImportOptions* options )
	{
		MeshImportOptions* meshOptions = options->ConstCast< MeshImportOptions >( );
		if ( !meshOptions )
		{
			return nullptr;
		}

		Assimp::Importer importer;		
	
		const aiScene* scene = importer.ReadFile( meshOptions->mResourceFilePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights ); 
		if ( !scene || !scene->mRootNode )
		{
			// Error 
			return nullptr;
		} 

		// Check whether or not the scene has animations
		bool createAnimation = meshOptions->mCreateAnimations;

		// Will construct the skeleton here if has bones and we want to create a new skeleton in the import process
		bool createSkeleton = meshOptions->mCreateSkeleton;

		// Detect whether or not scene has any mesh data
		bool createMesh = meshOptions->mCreateMesh; 

		// Skeleton asset to either create or reference
		AssetHandle< Skeleton > skeleton;

		// Mesh to return
		Mesh* mesh = nullptr;

		// Grab asset manager
		AssetManager* am = EngineSubsystem( AssetManager );

		if ( createMesh )
		{
			// Determine if is skeletal mesh
			if ( createSkeleton )
			{ 
				// Construct skeleton and create it
				SkeletonAssetLoader* sal = am->GetLoader( Object::GetClass< SkeletonAssetLoader >( ) )->ConstCast< SkeletonAssetLoader >();
				meshOptions->mSkeletonAsset = sal->DirectImport( meshOptions );
			}

			if ( meshOptions->mSkeletonAsset && createMesh )
			{
				SkeletalMeshAssetLoader* smal = am->GetLoader( Object::GetClass< SkeletalMeshAssetLoader >( ) )->ConstCast< SkeletalMeshAssetLoader >( );
				smal->DirectImport( meshOptions );
			} 
			// Otherwise static graphics mesh 
			else
			{
				// Construct new static mesh to fill out...
				mesh = new Mesh( );

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
		} 

		// If need to process animation
		if ( createAnimation )
		{
			SkeletalAnimationAssetLoader* saal = am->GetLoader( Object::GetClass< SkeletalAnimationAssetLoader >( ) )->ConstCast< SkeletalAnimationAssetLoader >( );
			saal->DirectImport( options );
		}

		// Reset mesh options
		meshOptions->Reset( );

		// Return mesh constructed ( or null )
		return mesh;
	}

	//============================================================================================================

	Asset* MeshAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Construct new mesh from filepath 
		Assimp::Importer importer;

		// NOTE(): Flipping UVs FUCKS IT ALL because I'm already flipping UVs in the shader generation process (shadergraph). Need to fix this.  
		const aiScene* scene = importer.ReadFile( filePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace );

		if ( !scene || !scene->mRootNode )
		{
			// Error 
		} 

		// Detect whether or not scene has any mesh data
		bool hasMesh = HasMesh( scene->mRootNode, scene );

		if ( hasMesh )
		{
			// Mesh to construct
			Mesh* mesh = new Mesh( ); 
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

		// No mesh created
		return nullptr; 
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

		// Iterate through tris and build vert list
		for ( u32 i = 0; i < aim->mNumFaces; ++i )
		{
			const aiFace& face = aim->mFaces[ i ];

			// Must have 3 indices per face ( tri )
			assert( face.mNumIndices == 3 );

			for ( u32 j = 0; j < 3; ++j )
			{
				u32 vertIdx = face.mIndices[ j ];

				sm->mVertexData.Write< f32 >( aim->mVertices[ vertIdx ].x );
				sm->mVertexData.Write< f32 >( aim->mVertices[ vertIdx ].y );
				sm->mVertexData.Write< f32 >( aim->mVertices[ vertIdx ].z );

				// Normal
				if ( aim->mNormals )
				{
					sm->mVertexData.Write< f32 >( aim->mNormals[ vertIdx ].x );
					sm->mVertexData.Write< f32 >( aim->mNormals[ vertIdx ].y );
					sm->mVertexData.Write< f32 >( aim->mNormals[ vertIdx ].z );
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
					sm->mVertexData.Write< f32 >( aim->mTangents[ vertIdx ].x );
					sm->mVertexData.Write< f32 >( aim->mTangents[ vertIdx ].y );
					sm->mVertexData.Write< f32 >( aim->mTangents[ vertIdx ].z ); 
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
					sm->mVertexData.Write< f32 >( aim->mTextureCoords[ 0 ][ vertIdx ].x );
					sm->mVertexData.Write< f32 >( aim->mTextureCoords[ 0 ][ vertIdx ].y );
				}
				else
				{
					sm->mVertexData.Write< f32 >( 0.0f );
					sm->mVertexData.Write< f32 >( 0.0f );
				}
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

	Result MeshAssetLoader::BeginImportInternal( const String& filePath, const String& cacheDirectory )
	{
		// Do things here...
		mImportOptions.mResourceFilePath = filePath;
		mImportOptions.mDestinationAssetDirectory = cacheDirectory; 

		// Construct new mesh from filepath 
		Assimp::Importer importer;

		// NOTE(): Flipping UVs FUCKS IT ALL because I'm already flipping UVs in the shader generation process (shadergraph). Need to fix this.  
		const aiScene* scene = importer.ReadFile( filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace );

		//if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
		if ( !scene || !scene->mRootNode )
		{
			// Error 
			return Result::FAILURE;
		} 

		// Check whether or not the scene has animations
		mImportOptions.mShowAnimationCreateDialogue = scene->HasAnimations( );
		// Will construct the skeleton here if has bones and we want to create a new skeleton in the import process
		mImportOptions.mShowSkeletonCreateDialogue = HasSkeleton( scene->mRootNode, scene );
		// Detect whether or not scene has any mesh data
		mImportOptions.mShowMeshCreateDialogue = HasMesh( scene->mRootNode, scene );

		return Result::SUCCESS;
	}

	//=====================================================================================================

	const ImportOptions* MeshAssetLoader::GetImportOptions( ) const
	{
		return &mImportOptions;
	} 

	//=====================================================================================================

	AssetHandle< Skeleton > MeshImportOptions::GetSkeleton( ) const
	{
		return mSkeletonAsset;
	}

	//=====================================================================================================

	void MeshImportOptions::Reset( )
	{
		mShowMeshCreateDialogue = false;
		mShowAnimationCreateDialogue = false;
		mShowSkeletonCreateDialogue = false;
		mSkeletonAsset = nullptr;
	}

	//=====================================================================================================

	Result MeshImportOptions::OnEditorUIInternal( )
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
		else
		{
			mCreateSkeleton = false;
		}

		if ( mShowMeshCreateDialogue )
		{ 
			bool createMesh = mCreateMesh;
			if ( igm->CheckBox( "Mesh", &createMesh ) )
			{
				mCreateMesh = createMesh;
			}
		} 
		else
		{
			mCreateMesh = false;
		}

		if ( mShowAnimationCreateDialogue )
		{
			bool createAnimations = mCreateAnimations;
			if ( igm->CheckBox( "Animation", &createAnimations ) )
			{
				mCreateAnimations = createAnimations;
			}
		}
		else
		{
			mCreateAnimations = false;
		}

		if ( mShowAnimationCreateDialogue && !mShowSkeletonCreateDialogue )   // However this would work...
		{
			if ( ImGui::BeginCombo( "##skeletons", ( mSkeletonAsset ? mSkeletonAsset->GetName() : "Skeleton..." ).c_str( ) ) )
			{
				// Grab all skeletons in database
				const HashMap< String, AssetRecordInfo >* skeletons = EngineSubsystem( AssetManager )->GetAssets< Skeleton >();	

				if ( skeletons )
				{
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

				ImGui::EndCombo( );
			}
		} 

		return Result::PROCESS_RUNNING;
	} 
	
	//=====================================================================================================

	String MeshAssetLoader::GetAssetFileExtension( ) const
	{
		return ".emsh";
	}

	//=====================================================================================================
} 

