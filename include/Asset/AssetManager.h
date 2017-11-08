// Copyright 2016-2017 John Jackson. All Rights Reserved.
// @file AssetManager.h
#pragma once
#ifndef ENJON_ASSET_MANAGER_H
#define ENJON_ASSET_MANAGER_H 

#include "Subsystem.h"
#include "System/Types.h"
#include "Asset/AssetLoader.h"
#include "Defines.h" 

// TODO(): For some reason, need this here. I hate this. Fix it.
#include "Graphics/Texture.h"

#include <array>

namespace Enjon
{
	class AssetLoader;
	class Asset; 

	class AssetManager : public Subsystem
	{
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
			*@brief Adds asset to project form given file path
			*/
			Result AddToDatabase( const String& filePath, b8 isRelativePath = true );
			
			/**
			*@brief Gets asset manager name
			*/
			Enjon::String GetName( );
			
			/**
			*@brief
			*/
			void SetAssetsPath( const String& filePath );
			
			/**
			*@brief
			*/
			Enjon::String GetAssetsPath( );
			
			/**
			*@brief
			*/
			void SetCachedAssetsPath( const String& filePath ); 
			
			/**
			*@brief
			*/
			const Enjon::String& GetCachedAssetsPath( ) const;
			
			/**
			*@brief
			*/
			void SetDatabaseName( const String& name );

			/**
			*@brief Gets loaded asset in database from name
			*/
			template <typename T>
			AssetHandle<T> GetAsset( const String& name ); 

			/**
			*@brief
			*/
			const Asset* GetAsset( const MetaClass* cls, const UUID& id );

			/**
			*@brief
			*/
			const HashMap< Enjon::String, AssetRecordInfo >* AssetManager::GetAssets( const Enjon::MetaClass* cls );

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
			const HashMap< String, AssetRecordInfo >* GetAssets( ); 

			/**
			*@brief Gets loaded default asset in database from name. Will load the asset if not currently available.
			*/
			Asset* GetDefaultAsset( const Enjon::MetaClass* cls );
			
			/**
			*@brief Gets loaded default asset in database by type id. Will load the asset if not currently available.
			*/
			template <typename T>
			AssetHandle<T> GetDefaultAsset( );
			
			/**
			*@brief Gets loader by meta class. If not found, returns nullptr.
			*/
			const AssetLoader* GetLoader( const MetaClass* cls );

			/**
			*@brief Gets loader by meta class of asset type. If not found, returns nullptr.
			*/
			template <typename T>
			const AssetLoader* GetLoaderByAssetType( );

			/**
			*@brief Gets loader by meta class of asset class. If not found, returns nullptr.
			*/
			const AssetLoader* GetLoaderByAssetClass( const MetaClass* cls );

			/**
			*@brief 
			*/
			static bool HasFileExtension( const String& file, const String& extension );

		protected:

		private: 

			/**
			*@brief
			*/
			s32 GetLoaderIdxByFileExtension(const String& filePath);

			/**
			*@brief
			*/
			u32 GetUniqueAssetTypeId( )
			{
				static u32 lastId { 0u };
				return lastId++; 
			}

			/**
			*@brief
			*/
			template <typename T>
			u32 GetAssetTypeId( ) noexcept;

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
			String mAssetsPath;
			String mCachedPath;
			String mName; 
	};

	#include "Asset/AssetManager.inl"
} 


#endif
