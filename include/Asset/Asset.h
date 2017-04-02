// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Asset.h

#pragma once
#ifndef ENJON_ASSET_H
#define ENJON_ASSET_H 

#include "Defines.h"
#include "System/Types.h"
#include "Base/Object.h"

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

			T* Get() { return mAsset->Cast<T>(); }

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
