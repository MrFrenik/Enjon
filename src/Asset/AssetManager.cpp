// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.cpp

#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "Asset/TextureAssetLoader.h" 
#include "Utils/FileUtils.h"

#include <fmt/printf.h>

namespace Enjon
{
	//=================================================

	AssetManager::AssetManager(const String& assetsPath)
	{
		// Set project asset path
		mAssetsPath = assetsPath;

		// Register the loaders with manager 
		RegisterAssetLoader<Enjon::Texture, TextureAssetLoader>(); 
	}
	
	//=================================================
			
	AssetManager::~AssetManager()
	{ 
		// Release all loaders from memory
		for (auto& loader : mLoaders)
		{
			delete loader.second;
		}

		// Clear map
		mLoaders.clear();
	}
			
	Result AssetManager::Initialize()
	{

		return Result::SUCCESS;
	}

	AssetManager::LoaderType AssetManager::GetTypeByFileExtension(const String& filePath)
	{ 
		LoaderType type;

		String fileExtension = Utils::SplitString(filePath, ".").back();
			
		fmt::print("fileExtension: {}\n", fileExtension);

		if (fileExtension.compare("png") == 0 )
		{ 
			type = LoaderType::Texture;
		}

		return type;
	}
	
	//================================================= 
			
	Result AssetManager::AddToDatabase(const String& filePath)
	{
		// Have to do a switch based on extension of file
		LoaderType fType = GetTypeByFileExtension(filePath);

		// Find loader by type
		auto query = mLoaders.find((u32)fType);
		if (query != mLoaders.end())
		{
			// Load asset and place into database
			auto res = query->second->LoadAssetFromFile(mAssetsPath + filePath); 
		}

		return Result::SUCCESS;
	} 
}







