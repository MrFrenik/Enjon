// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Asset.cpp

#include "Asset/Asset.h" 
#include "Asset/AssetManager.h"
#include "Utils/FileUtils.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetLoader.h"
#include "Engine.h"
#include "SubsystemCatalog.h"
			
namespace Enjon
{ 
	//================================================================================================================

	template <typename T>
	const AssetLoader* AssetHandle<T>::GetLoader( ) const
	{
		Engine::GetInstance( )->GetSubsystemCatalog( )->Get<AssetManager>( )->GetLoaderByAssetType( Object::GetClass<T>( ) );
	} 

	//================================================================================================================

	const AssetRecordInfo* Asset::GetAssetRecordInfo( ) const
	{
		return mRecordInfo;
	}

	//================================================================================================================

	Result Asset::Save( ) const
	{
		return Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->SaveAsset( this );
	}

	//================================================================================================================

	bool Asset::IsDefault( ) const
	{
		return mIsDefault;
	} 

	//================================================================================================================ 
}
