
#include "Asset/SkeletalAnimationAssetLoader.h"

namespace Enjon
{
	//======================================================================

	SkeletalAnimationAssetLoader::SkeletalAnimationAssetLoader( )
	{ 
	}

	//======================================================================

	SkeletalAnimationAssetLoader::~SkeletalAnimationAssetLoader( )
	{ 
	}

	//======================================================================

	void SkeletalAnimationAssetLoader::RegisterDefaultAsset( )
	{ 
	}

	//======================================================================

	Asset* SkeletalAnimationAssetLoader::LoadResourceFromFile( const String& filePath )
	{ 
		// Create animation - return it
		SkeletalAnimation* anim = new SkeletalAnimation( );

		return anim;
	}

	//======================================================================
}

