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

	ENJON_CLASS( Construct )
	class SkeletalMesh : public Mesh
	{
		friend SkeletalMeshAssetLoader;

		ENJON_CLASS_BODY( )

		public:
			/*
			* @brief Constructor
			*/
			SkeletalMesh();

			/*
			* @brief Destructor
			*/
			~SkeletalMesh(); 

		public:
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

		protected: 
			
			AssetHandle< Skeleton > mSkeleton;
	}; 
}


#endif
