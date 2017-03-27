// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.cpp

#include "Asset/AssetManager.h"
#include "Asset/AssetLoader.h"
#include "Asset/TextureAssetLoader.h" 
#include "Asset/MeshAssetLoader.h" 
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
		RegisterAssetLoader<Enjon::Mesh, MeshAssetLoader>(); 
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

	s32 AssetManager::GetLoaderIdxByFileExtension(const String& filePath)
	{ 
		// If not found, will return -1
		s32 idx = -1;

		String fileExtension = Utils::SplitString(filePath, ".").back(); 

		// Graphics texture asset
		if (fileExtension.compare("png") == 0 )
		{ 
			idx = GetAssetTypeId<Enjon::Texture>();
		}

		// Graphics mesh asset
		else if (fileExtension.compare("obj") == 0)
		{
			idx = GetAssetTypeId<Enjon::Mesh>();
		}

		return idx;
	}
	
	//================================================= 
			
	Result AssetManager::AddToDatabase(const String& filePath, b8 isRelativePath)
	{
		// Have to do a switch based on extension of file
		s32 idx = GetLoaderIdxByFileExtension(filePath); 

		// If out of bounds, return failure since file extension was unknown
		if (idx < 0) 
		{
			// TODO(): Log that extension was unknown
			return Result::FAILURE;
		}
		
		// Get qualified name of asset
		String qualifiedName = AssetLoader::GetQualifiedName(filePath); 

		// Find loader by idx
		auto query = mLoaders.find((u32)idx);
		if (query != mLoaders.end())
		{
			// Make sure it doesn't exist already before trying to load it
			if (query->second->Exists(qualifiedName))
			{
				return Result::FAILURE;
			}
			else
			{
				// Load asset and place into database
				auto res = query->second->LoadAssetFromFile(mAssetsPath + filePath, qualifiedName); 
			}
		}

		return Result::SUCCESS;
	} 
}







