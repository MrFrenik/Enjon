// @file ImportOptions.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.  

#include "Asset/ImportOptions.h"

namespace Enjon 
{ 
	//============================================================================

	String ImportOptions::GetResourceFilePath( ) const
	{
		return mResourceFilePath;
	}

	//============================================================================

	String ImportOptions::GetDestinationAssetDirectory( ) const
	{
		return mDestinationAssetDirectory;
	}

	//============================================================================

	bool ImportOptions::IsImporting( ) const
	{
		return mIsImporting;
	} 

	//============================================================================
}

