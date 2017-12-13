// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetManager.inl

//================================================================================================
			
template <typename T, typename K>
Enjon::Result AssetManager::RegisterAssetLoader()
{
	static_assert(std::is_base_of<Asset, T>::value,
		"RegisterAssetLoader: T must inherit from Asset.");
	
	static_assert(std::is_base_of<AssetLoader, K>::value,
		"RegisterAssetLoader: K must inherit from AssetLoader.");

	// Get idx of asset loader type
	u32 idx = GetAssetTypeId<T>(); 

	// Register internally with system
	Enjon::Result res = RegisterAssetLoaderInternal( new K( ), idx ); 

	return res;
}

//================================================================================================

template <typename T>
u32 AssetManager::GetAssetTypeId( ) const noexcept
{
	static_assert( std::is_base_of<Asset, T>::value, "GetAssetTypeId:: T must inherit from Asset." );

	// Return id type from object
	return Enjon::Object::GetTypeId< T >( );

	static u32 typeId{ GetUniqueAssetTypeId( ) };
	return typeId;
}

//================================================================================================ 

template <typename T>
AssetHandle<T> AssetManager::GetDefaultAsset( ) const
{
	// Get appropriate loader based on asset type
	u32 loaderId = GetAssetTypeId<T>( );

	// Get handle from loader
	Asset* defaultAsset = ConstCast< AssetManager >()->mLoadersByAssetId[loaderId]->GetDefault( );
	AssetHandle<T> handle = AssetHandle<T>( defaultAsset );

	// Return asset handle
	return handle;
}

//================================================================================================ 

template <typename T>
const HashMap< String, AssetRecordInfo >* AssetManager::GetAssets( ) const
{
	// Get appropriate loader based on asset type
	u32 loaderId = GetAssetTypeId<T>( );

	if ( Exists( loaderId ) )
	{
		return ConstCast< AssetManager >()->mLoadersByAssetId[loaderId]->GetAssets( );
	}

	return nullptr;
} 

//================================================================================================ 

template <typename T>
AssetHandle<T> AssetManager::GetAsset( const String& name ) const
{
	// Get appropriate loader based on asset type
	u32 loaderId = GetAssetTypeId<T>( );

	// Get handle from loader
	AssetHandle<T> handle = ConstCast< AssetManager >()->mLoadersByAssetId[loaderId]->GetAsset( name );

	// Return asset handle
	return handle;
}

//================================================================================================ 

template <typename T>
AssetHandle<T> AssetManager::GetAsset( const UUID& uuid ) const
{
	// Get appropriate loader based on asset type
	u32 loaderId = GetAssetTypeId<T>( );

	// Get handle from loader
	AssetHandle<T> handle = ConstCast< AssetManager >()->mLoadersByAssetId[loaderId]->GetAsset( uuid );

	// Return asset handle
	return handle;
}

//================================================================================================ 

template <typename T>
const AssetLoader* AssetManager::GetLoaderByAssetType( ) const
{
	// Get appropriate loader based on asset type
	u32 loaderId = GetAssetTypeId<T>( );

	// If exists, return it
	if ( Exists( loaderId ) )
	{
		return ConstCast< AssetManager >()->mLoadersByAssetId[ loaderId ];
	}

	return nullptr; 
}

//================================================================================================ 

template <typename T>
AssetHandle< T > AssetManager::ConstructAsset( )
{ 
	// Get the asset loader
	const AssetLoader* loader = GetLoaderByAssetType< T >( );

	// Construct new asset and get handle to it
	AssetHandle< T > handle = const_cast< AssetLoader* >( loader )->ConstructAsset< T >( this );

	// Serialize the asset to file
	SerializeAsset( handle.Get( ) );

	return handle; 
}









