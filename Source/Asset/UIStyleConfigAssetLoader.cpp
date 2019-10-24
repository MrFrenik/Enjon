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

	String UIStyleSheetAssetLoader::GetAssetFileExtension() const
	{
		return ".ess";
	}

	//======================================================================= 
			
	void UIStyleSheetAssetLoader::RegisterDefaultAsset()
	{
		// Create new graph and compile
		UIStyleSheet* styleSheet = new UIStyleSheet( );
		styleSheet->mName = "DefaultStyleSheet";

		// Set default asset
		mDefaultAsset = styleSheet; 
	}

	//======================================================================= 

	Asset* UIStyleSheetAssetLoader::LoadResourceFromFile( const String& filePath )
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

