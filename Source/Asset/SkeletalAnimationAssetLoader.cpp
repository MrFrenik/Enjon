// @file SkeletalAnimationAssetLoader.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Asset/SkeletalAnimationAssetLoader.h" 
#include "Asset/MeshAssetLoader.h"
#include "Graphics/Skeleton.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Enjon
{ 
	//======================================================================

	void SkeletalAnimationAssetLoader::RegisterDefaultAsset( )
	{
		mDefaultAsset = new SkeletalAnimation( );
	}

	//======================================================================

	Asset* SkeletalAnimationAssetLoader::DirectImport( const ImportOptions* options )
	{
		// Need to basically act as the asset manager here...
		SkeletalAnimation* animation = this->LoadResourceFromImporter( options )->ConstCast< SkeletalAnimation >( );

		if ( animation )
		{
			// Register, cache asset
			MeshImportOptions mo = *options->ConstCast< MeshImportOptions >( ); 
			mo.mLoader = this; 
			EngineSubsystem( AssetManager )->AddToDatabase( animation, &mo );
		}

		// Return mesh after processing
		return animation;
	}

	//======================================================================

	Asset* SkeletalAnimationAssetLoader::LoadResourceFromImporter( const ImportOptions* options )
	{
		MeshImportOptions* meshOptions = options->ConstCast< MeshImportOptions >( );
		if ( !meshOptions )
		{
			return nullptr;
		}

		// Construct new mesh from filepath 
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile( meshOptions->GetResourceFilePath( ), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace );
		if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
		{
			// Error 
			//return nullptr;
		}

		// Grab skeleton asset to use for this animation from import options
		AssetHandle< Skeleton > skeleton = meshOptions->GetSkeleton( );
		if ( !skeleton )
		{
			return nullptr;
		}

		// Skeleton to create
		SkeletalAnimation* animation = new SkeletalAnimation( );

		// Process skeleton
		ProcessAnimNode( scene->mRootNode, scene, skeleton, animation );

		// Set the skeleton reference
		animation->mSkeleton = skeleton;

		// Return skeleton after processing
		return animation;
	}

	//======================================================================

	void SkeletalAnimationAssetLoader::ProcessAnimNode( aiNode* node, const aiScene* scene, const AssetHandle< Skeleton >& skeletonHandle, SkeletalAnimation* animation )
	{
		// Grab skeleton asset from handle
		const Skeleton* skeleton = skeletonHandle.Get( );

		aiAnimation* aiAnim = scene->mAnimations[ 0 ];

		// Resize channel data for animation
		animation->mChannelData.resize( skeleton->GetNumberOfJoints() );

		// Set ticks per second for the animation
		animation->mTicksPerSecond = aiAnim->mTicksPerSecond;

		// Set duration in ticks for animation
		animation->mNumberOfTicks = aiAnim->mDuration;

		Vector< aiNodeAnim* > aiAnimNodes;

		for ( u32 i = 0; i < aiAnim->mNumChannels; ++i )
		{
			aiNodeAnim* animNode = aiAnim->mChannels[ i ];
			String channelName = animNode->mNodeName.C_Str(); 

			// Get index of joint
			s32 jointID = skeleton->GetJointIndex( channelName ); 

			// Make sure that joint id is valid
			if ( jointID < 0 )
			{
				// Error...
				std::cout << "Joint node found on skeleton: " + channelName + "\n";
				continue; 
			}

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

	//======================================================================

	String SkeletalAnimationAssetLoader::GetAssetFileExtension( ) const
	{
		return ".eanm";
	}

	//=======================================================================================
}

