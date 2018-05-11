// @file SkeletalMeshRenderable.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Graphics/SkeletalMeshRenderable.h"
#include "Graphics/AnimationSubsystem.h"
#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//===================================================================================

	void SkeletalMeshRenderable::SetMesh( const Mesh* mesh )
	{
		mMesh = mesh;

		// Make sure that material element vector matches amount of submeshes
		u32 subMeshCount = mMesh->GetSubMeshCount( );

		// Early out if equal
		if ( mMaterialElements.size( ) == subMeshCount )
		{
			return;
		}

		// If there were too many materials, then iterate up to mesh count
		if ( mMaterialElements.size( ) > subMeshCount )
		{
			Vector<AssetHandle<Material>> newMats; 
			for ( u32 i = 0; i < subMeshCount; ++i )
			{
				newMats.push_back( mMaterialElements.at( i ) );
			}

			// Set materials
			mMaterialElements = newMats;
		}
		// Otherwise there weren't enough, so iterate up to material elements
		else
		{
			u32 diff = subMeshCount - mMaterialElements.size( );
			Vector<AssetHandle<Material>> newMats; 
			for ( u32 i = 0; i < mMaterialElements.size(); ++i )
			{
				newMats.push_back( mMaterialElements.at( i ) );
			}

			AssetManager* am = EngineSubsystem( AssetManager );

			for ( u32 i = 0; i < diff; ++i )
			{
				newMats.push_back( am->GetDefaultAsset< Material >( ) );
			} 

			// Set materials
			mMaterialElements = newMats; 
		}

	}

	//===================================================================================

	const Mesh* SkeletalMeshRenderable::GetMesh( ) const
	{
		return mMesh.Get( );
	}

	//===================================================================================

	void SkeletalMeshRenderable::SetMesh( const AssetHandle< SkeletalMesh >& mesh )
	{
		mMesh = mesh;

		// Make sure that material element vector matches amount of submeshes
		u32 subMeshCount = mMesh->GetSubMeshCount( );

		// Early out if equal
		if ( mMaterialElements.size( ) == subMeshCount )
		{
			return;
		}

		// If there were too many materials, then iterate up to mesh count
		if ( mMaterialElements.size( ) > subMeshCount )
		{
			Vector<AssetHandle<Material>> newMats; 
			for ( u32 i = 0; i < subMeshCount; ++i )
			{
				newMats.push_back( mMaterialElements.at( i ) );
			}

			// Set materials
			mMaterialElements = newMats;
		}
		// Otherwise there weren't enough, so iterate up to material elements
		else
		{
			u32 diff = subMeshCount - mMaterialElements.size( );
			Vector<AssetHandle<Material>> newMats; 
			for ( u32 i = 0; i < mMaterialElements.size(); ++i )
			{
				newMats.push_back( mMaterialElements.at( i ) );
			}

			AssetManager* am = EngineSubsystem( AssetManager );

			for ( u32 i = 0; i < diff; ++i )
			{
				newMats.push_back( am->GetDefaultAsset< Material >( ) );
			} 

			// Set materials
			mMaterialElements = newMats; 
		}
	}

	//=================================================================================== 

	AssetHandle< Skeleton > SkeletalMeshRenderable::GetSkeleton( ) const 
	{
		return mMesh.Get( )->GetSkeleton( );
	}

	//=================================================================================== 

	const Vector< Mat4x4 >& SkeletalMeshRenderable::GetJointTransforms( ) const
	{
		return mFinalJointTransforms;
	}

	//=================================================================================== 
}
