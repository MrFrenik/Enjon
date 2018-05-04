#include "Graphics/SkeletalMesh.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//=============================================================================

	SkeletalMesh::SkeletalMesh( )
	{ 
	}

	//=============================================================================

	SkeletalMesh::~SkeletalMesh( )
	{ 
	}

	//=============================================================================

	Result SkeletalMesh::SerializeData( ByteBuffer* buffer ) const
	{
		// Serialize base
		Mesh::SerializeData( buffer );

		// Serialize skeleton asset reference
		buffer->Write< UUID >( mSkeleton.GetUUID() );

		return Result::SUCCESS;
	}

	//=============================================================================

	Result SkeletalMesh::DeserializeData( ByteBuffer* buffer )
	{
		// Deserialize base
		Mesh::DeserializeData( buffer );

		// Deserialize skeleton asset
		mSkeleton = EngineSubsystem( AssetManager )->GetAsset<Skeleton>( buffer->Read< UUID >( ) );

		return Result::SUCCESS;
	}

	//=============================================================================
}
