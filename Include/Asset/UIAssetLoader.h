// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: UIAssetLoader.h

#pragma  once
#ifndef ENJON_UI_ASSET_LOADER_H
#define ENJON_UI_ASSET_LOADER_H 

#include "GUI/UIAsset.h"
#include "Asset/AssetLoader.h"

namespace Enjon
{
	ENJON_CLASS( )
	class UIAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( UIAssetLoader )
		
		public: 

			/**
			* @brief Destructor
			*/
			virtual String GetAssetFileExtension( ) const override;

		protected:

			/**
			* @brief
			*/
			virtual void RegisterDefaultAsset( ) override; 

		private:
			/**
			* @brief
			*/
			Asset* LoadResourceFromFile( const String& filePath ) override; 
	}; 
}

#endif
