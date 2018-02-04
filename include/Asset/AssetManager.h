// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file AssetManager.h
#pragma once
#ifndef ENJON_ASSET_MANAGER_H
#define ENJON_ASSET_MANAGER_H 

#include "Subsystem.h"
#include "System/Types.h"
#include "Asset/AssetLoader.h"
#include "Serialize/CacheRegistryManifest.h"
#include "Defines.h" 
#include "Engine.h"

#include <array>

namespace Enjon
{
	class AssetLoader;
	class Asset; 

	ENJON_CLASS( )
	class AssetManager : public Subsystem
	{
		ENJON_CLASS_BODY( )

		enum class LoaderType
		{
			Texture,
			Material,
			Mesh,
			Shader,
			Font,
			Count
		}; 

		public: 
			
			/**
			*@brief Constructor
			*/
			AssetManager();

			/**
			*@brief Constructor
			*/
			AssetManager(const String& name, const String& assetsPath);

			/**
			*@brief Virtual destructor
			*/
			~AssetManager();

			/**
			*@brief
			*/
			virtual Result Initialize();

			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) override;

			/**
			*@brief
			*/
			virtual Result Shutdown();

			/**
			*@brief
			*/
			void Reinitialize( const String& assetsPath );
	
			/**
			*@brief Adds asset to project form given file path
			*/
			Result AddToDatabase( const String& filePath, bool cache = true, bool isRelativePath = true );

			/*
			*@brief
			*/
			Result SaveAsset( const Asset* asset ) const;

			/**
			*@brief
			*/
			AssetLocationType GetAssetLocationType( ) const;

			/*
			* @brief Creates a new asset of given type, adds it into the database and returns a handle it
			*/
			//AssetHandle< Asset > ConstructAsset( const MetaClass* assetCls );

			/*
			* @brief Creates a new asset of given type, adds it into the database and returns a handle it
			*/
			template <typename T>
			AssetHandle< T > ConstructAsset( const String& assetName = "" );

			/**
			*@brief Gets asset manager name
			*/
			Enjon::String GetName( );
			
			/**
			*@brief
			*/
			void SetAssetsDirectoryPath( const String& filePath );
			
			/**
			*@brief
			*/
			Enjon::String GetAssetsDirectoryPath( ) const;
			
			/**
			*@brief
			*/
			void SetCachedAssetsDirectoryPath( const String& filePath ); 
			
			/**
			*@brief
			*/
			const Enjon::String& GetCachedAssetsDirectoryPath( ) const;
			
			/**
			*@brief
			*/
			void SetDatabaseName( const String& name );

			/**
			*@brief Gets loaded asset in database from name
			*/
			template <typename T>
			AssetHandle<T> GetAsset( const String& name ) const; 

			/**
			*@brief Gets loaded asset in database from uuid
			*/
			template <typename T>
			AssetHandle<T> GetAsset( const UUID& uuid ) const; 

			/**
			*@brief
			*/
			const Asset* GetAsset( const MetaClass* cls, const UUID& id ) const; 

			/**
			*@brief
			*/
			const HashMap< Enjon::String, AssetRecordInfo >* AssetManager::GetAssets( const Enjon::MetaClass* cls ) const;

			/**
			*@brief Searches for specific loader based on class id. Returns true if found, false otherwise.
			*/
			bool Exists( u32 id ) const;

			/**
			*@brief Searches for specific loader based on meta class. Returns true if found, false otherwise.
			*/
			bool Exists( const MetaClass* cls ) const;
			
			/**
			*@brief Gets all assets of specific type
			*/
			template <typename T>
			const HashMap< String, AssetRecordInfo >* GetAssets( ) const; 

			/**
			*@brief Gets loaded default asset in database from name. Will load the asset if not currently available.
			*/
			Asset* GetDefaultAsset( const Enjon::MetaClass* cls ) const;
			
			/**
			*@brief Gets loaded default asset in database by type id. Will load the asset if not currently available.
			*/
			template <typename T>
			AssetHandle<T> GetDefaultAsset( ) const;
			
			/**
			*@brief Gets loader by meta class. If not found, returns nullptr.
			*/
			const AssetLoader* GetLoader( const MetaClass* cls ) const;

			/**
			*@brief Gets loader by meta class of asset type. If not found, returns nullptr.
			*/
			template <typename T>
			const AssetLoader* GetLoaderByAssetType( ) const;

			/**
			*@brief Gets loader by meta class of asset class. If not found, returns nullptr.
			*/
			const AssetLoader* GetLoaderByAssetClass( const MetaClass* cls ) const;

			/**
			*@brief 
			*/
			static bool HasFileExtension( const String& file, const String& extension );

		protected:

		private: 

			/**
			*@brief Adds asset to project form given file path
			*/
			Result SerializeAsset( const Asset* asset );

			/**
			*@brief
			*/
			s32 GetLoaderIdxByFileExtension(const String& filePath);

			/**
			*@brief
			*/
			u32 GetUniqueAssetTypeId( ) const
			{
				static u32 lastId { 0u };
				return lastId++; 
			}

			/**
			*@brief
			*/
			template <typename T>
			u32 GetAssetTypeId( ) const noexcept;

			/**
			*@brief Used internally by AssetManager to place appropriate loaders into associative maps. 
			*@note NOT TO BE DIRECTLY CALLED. 
			*/
			Enjon::Result RegisterAssetLoaderInternal( AssetLoader* loader, u32 idx );

			/**
			*@brief
			*/
			template <typename T, typename K>
			Enjon::Result RegisterAssetLoader( );

		private:
			void RegisterLoaders( );

		private: 

			std::unordered_map< const MetaClass*, AssetLoader* > mLoadersByMetaClass;
			std::unordered_map<u32, AssetLoader*> mLoadersByAssetId; 
			std::unordered_map< Enjon::String, u32 > mFileExtensionMap;
			String mAssetsDirectoryPath;
			String mCachedDirectoryPath;
			String mName; 
			CacheRegistryManifest mCacheManifest;
			AssetLocationType mAssetLocationType = AssetLocationType::EngineAsset;
	};

	#include "Asset/AssetManager.inl"
} 


#endif
