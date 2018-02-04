// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: CacheRegistryManifest.h

#pragma once
#ifndef ENJON_CACHE_REGISTRY_MANIFEST_H
#define ENJON_CACHE_REGISTRY_MANIFEST_H 

#include "System/Types.h"
#include "Defines.h"
#include "Serialize/UUID.h"
#include "Serialize/ByteBuffer.h"
#include "Base/Object.h"

namespace Enjon
{ 
	enum class AssetLocationType
	{
		EngineAsset,
		ApplicationAsset
	};

	struct CacheManifestRecord
	{
		UUID mAssetUUID							= UUID::Invalid();
		String mAssetFilePath					= "";
		String mAssetName						= "";
		AssetLocationType mAssetLocationType	= AssetLocationType::ApplicationAsset;
		const MetaClass* mAssetLoaderClass		= nullptr;
	};

	class CacheRegistryManifest
	{
		friend AssetManager;

		public:
			/*
			* @brief Default constructor
			*/
			CacheRegistryManifest( ) = default; 

			/*
			* @brief Default destructor
			*/
			~CacheRegistryManifest( ) = default; 

		protected: 

			/*
			* @brief
			*/
			void Reset( );

			/*
			* @brief
			*/
			Result Initialize( const String& manifestPath, const AssetManager* manager ); 

			/*
			* @brief
			*/
			Result AddRecord( const CacheManifestRecord& record );

		private:

			/*
			* @brief
			*/
			Result WriteOutManifest( const String& manifestPath );

			/*
			* @brief
			* @note Will be read in ONLY by calling Initialize() first, since the manifest path must be given
			*/
			Result ReadInManifest( ); 

			/*
			* @brief
			*/
			bool HasRecord( const UUID& uuid );

		private:
			String mManifestPath; 
			HashMap< String, CacheManifestRecord > mManifestRecords;
			const AssetManager* mAssetManager = nullptr;
	};
}

#endif
