// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetLoader.cpp

#include "Asset/AssetLoader.h"
#include "Utils/FileUtils.h"

namespace Enjon
{ 
	//=================================================================

	AssetLoader::AssetLoader()
	{ 
	}

	//=================================================================

	AssetLoader::~AssetLoader()
	{
		// Delete all associated assets
		for (auto& asset : mAssets)
		{
			delete asset.second;
		}

		mAssets.clear();
	}

	//=================================================================

	String AssetLoader::GetQualifiedName(const String& filePath)
	{
		std::vector<String> splits = Enjon::Utils::SplitString(filePath, "."); 
		String res = Enjon::Utils::Replace(splits.at(0), '/', '.');
		return Enjon::Utils::ToLower(res);
	} 
	
	//=================================================================
			
	b8 AssetLoader::Exists(const String& name)
	{
		auto query = mAssets.find(name);
		if (query != mAssets.end())
		{
			return true;
		}

		return false;
	}
}

