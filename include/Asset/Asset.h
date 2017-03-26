// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Asset.h

#pragma once
#ifndef ENJON_ASSET_H
#define ENJON_ASSET_H 

#include "Defines.h"
#include "System/Types.h"

#include <assert.h>
#include <memory>

namespace Enjon
{ 
	class AssetLoader;
	class AssetManager;

	class Asset
	{
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

		template <typename T>
		T* SafeCast()
		{
			static_assert(std::is_base_of<Asset, T>::value, 
				"SafeCast:: T must inherit from Asset.");	

			return static_cast<T*>(this);
		}

		protected:

			

		private:
	};
	
	template <typename T>
	class AssetHandle
	{
		public:
			AssetHandle() 
			{
				static_assert(std::is_base_of<Asset, T>::value, 
					"AssetHandle:: T must inherit from Asset.");	
			}

			AssetHandle(Asset* asset)
			{
				static_assert(std::is_base_of<Asset, T>::value, 
					"AssetHandle:: T must inherit from Asset.");	

				mAsset = asset;
			}

			T* Get() { return mAsset->SafeCast<T>(); }

			b8 IsValid()
			{
				return (mAsset != nullptr);
			}
			
			Result Set(Asset* asset)
			{
				// Set to new asset
				mAsset = asset;

				return Result::SUCCESS;
			}

		protected:

		private: 
			Asset* mAsset = nullptr;
	};
} 

#endif
