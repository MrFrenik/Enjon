// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Asset.h

#pragma once
#ifndef ENJON_ASSET_H
#define ENJON_ASSET_H 

#include "Defines.h"
#include "System/Types.h"
#include "Base/Object.h"
#include "Serialize/UUID.h"
#include "Serialize/ByteBuffer.h"
#include "Asset/AssetUtils.h"

#include <assert.h>
#include <memory>

namespace Enjon
{ 
	class AssetLoader;
	class AssetManager; 

	ENJON_CLASS( )
	class Asset : public Enjon::Object
	{
		ENJON_CLASS_BODY( )

		public: 
			friend AssetLoader;
			friend AssetManager;

			/**
			*@brief Constructor
			*/
			Asset() {}

			/**
			*@brief Virtual destructor
			*/
			~Asset() {} 

			/**
			*@brief
			*/
			ENJON_FUNCTION( )
			Enjon::String GetName( ) const { return mName; }
			
			/**
			*@brief
			*/
			ENJON_FUNCTION( )
			Enjon::String GetFilePath( ) const { return mFilePath; }

			/**
			*@brief
			*/
			ENJON_FUNCTION( )
			UUID GetUUID( ) const { return mUUID; } 

			/**
			*@brief
			*/
			const AssetLoader* GetLoader( ) const { return mLoader; }
			
		protected:

			/*
			* @brief Caches file into cache directory relative to project directory 
			*/
			virtual Result CacheFile( Enjon::ByteBuffer& buffer )
			{ 
				return Result::SUCCESS;
			} 

			/*
			* @brief Caches file into cache directory relative to project directory 
			*/
			virtual Result Serialize( Enjon::ByteBuffer& buffer )
			{
				return Result::SUCCESS;
			}

			/*
			* @brief Caches file into cache directory relative to project directory 
			*/
			virtual Result Deserialize( Enjon::ByteBuffer& buffer )
			{
				return Result::SUCCESS;
			}

		public:
			/*
			* @brief
			*/
			virtual Result Reload( )
			{ 
				return Result::SUCCESS;
			} 

		protected: 

			ENJON_PROPERTY( )
			UUID mUUID;

			ENJON_PROPERTY( )
			Enjon::String mFilePath;

			ENJON_PROPERTY( )
			Enjon::String mName;

			const AssetLoader* mLoader = nullptr;

		private:
	};

	template <typename T> 
	class AssetHandle
	{
		public:
			/*
			* @brief Constructor
			*/
			AssetHandle() 
			{
				static_assert(std::is_base_of<Asset, T>::value, "AssetHandle:: T must inherit from Asset.");	
			}

			/*
			* @brief Constructor
			*/
			AssetHandle(Asset* asset)
			{
				static_assert(std::is_base_of<Asset, T>::value, "AssetHandle:: T must inherit from Asset.");	

				mAsset = asset;
			} 

			/*
			* @brief Destructor
			*/
			~AssetHandle( )
			{ 
			}
			
			/*
			* @brief
			*/
			explicit operator bool( ) 
			{
				return IsValid( );
			} 

			/*
			* @brief
			*/
			UUID GetUUID( )
			{
				// Return UUID if valid
				if ( IsValid( ) )
				{
					return mAsset->GetUUID();
				}

				return UUID::Invalid( );
			}

			/*
			* @brief
			*/
			T* Get() 
			{ 
				return mAsset->Cast<T>(); 
			}

			/*
			* @brief
			*/
			const MetaClass* GetAssetClass( ) 
			{ 
				return Object::GetClass<T>( );
			}
			
			/*
			* @brief
			*/
			T* operator->( )
			{
				return Get( );
			} 

			/*
			* @brief 
			*/
			bool IsValid()
			{
				return (mAsset != nullptr);
			} 
			
			/*
			* @brief 
			*/
			Result Set( Asset* asset ) 
			{
				// Set to new asset
				mAsset = asset;

				// Return success
				return Result::SUCCESS;
			} 

			/*
			* @brief Gets loader based on asset loader type
			*/
			const AssetLoader* GetLoader( ) const;

		protected:

		private: 
			Asset* mAsset = nullptr;
	};
} 

#endif
