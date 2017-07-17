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

#include <assert.h>
#include <memory>

namespace Enjon
{ 
	class AssetLoader;
	class AssetManager; 

	class Asset : public Enjon::Object
	{
		ENJON_OBJECT( Asset )

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
			Enjon::String GetName( ) const { return mName; }
			
			/**
			*@brief
			*/
			Enjon::String GetFilePath( ) const { return mFilePath; }

			
		protected:
			/*
			* @brief Caches file into cache directory relative to project directory 
			*/
			virtual Result CacheFile( Enjon::ByteBuffer& buffer )
			{ 
				return Result::SUCCESS;
			}

		protected: 
			UUID mUUID;
			Enjon::String mFilePath;
			Enjon::String mName;

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
				static_assert(std::is_base_of<Asset, T>::value, 
					"AssetHandle:: T must inherit from Asset.");	
			}

			/*
			* @brief Constructor
			*/
			AssetHandle(Asset* asset)
			{
				static_assert(std::is_base_of<Asset, T>::value, 
					"AssetHandle:: T must inherit from Asset.");	

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
			T* Get() 
			{ 
				return mAsset->Cast<T>(); 
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
			Result Set(Asset* asset)
			{
				// Set to new asset
				mAsset = asset;

				// Return success
				return Result::SUCCESS;
			}

		protected:

		private: 
			Asset* mAsset = nullptr;
	};
} 

#endif
