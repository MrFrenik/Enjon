#pragma once
#ifndef ENJON_SKELETAL_MESH_H
#define ENJON_SKELETAL_MESH_H

#include "Graphics/Skeleton.h"
#include "Graphics/Mesh.h"

namespace Enjon 
{ 
	// Forward Declarations
	class SubMesh;
	class Mesh;
	class SkeletalMeshAssetLoader; 

	ENJON_CLASS( )
	class SkeletalMesh : public Mesh
	{
		friend SkeletalMeshAssetLoader;

		ENJON_CLASS_BODY( SkeletalMesh ) 

		public:
			/**
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/**
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

			/**
			* @brief
			*/
			AssetHandle< Skeleton > GetSkeleton( ) const;

		protected: 
			
			ENJON_PROPERTY( HideInEditor )
			AssetHandle< Skeleton > mSkeleton;
	}; 
}


#endif
