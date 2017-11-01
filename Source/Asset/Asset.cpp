// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Asset.cpp

#include "Asset/Asset.h" 
#include "Asset/AssetManager.h"
#include "Utils/FileUtils.h"
#include "Engine.h"
			
namespace Enjon
{
	/*
	* @brief Gets loader based on asset loader type
	*/
	template <typename T>
	const AssetLoader* AssetHandle<T>::GetLoader( ) const
	{
		Engine::GetInstance( )->GetSubsystemCatalog( )->Get<AssetManager>( )->GetLoaderByAssetType( Object::GetClass<T>( ) );
	}
}
