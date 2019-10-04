#include "Asset/UIStyleConfigAssetLoader.h"

namespace Enjon
{
	//=======================================================================
	
	String UIStyleConfigAssetLoader::GetAssetFileExtension() const
	{
		return ".euicfg";
	}

	//=======================================================================
	
	void UIStyleConfigAssetLoader::RegisterDefaultAsset()
	{ 
		// Create new graph and compile
		UIStyleConfig* styleConfig = new UIStyleConfig( );
		styleConfig->mName = "DefaultUIStyleConfig";

		// Set default asset
		mDefaultAsset = styleConfig; 
	}

	//=======================================================================

	Asset* UIStyleConfigAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		// Just return the default material asset
		if ( !mDefaultAsset )
		{
			RegisterDefaultAsset( );
		}

		return mDefaultAsset;
	}

	//======================================================================= 
}

