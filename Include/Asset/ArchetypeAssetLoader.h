// File: Archetype.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma  once
#ifndef ENJON_ARCHETYPE_ASSET_LOADER_H
#define ENJON_ARCHETYPE_ASSET_LOADER_H 

#include "Entity/Archetype.h"
#include "Asset/AssetLoader.h"

namespace Enjon
{
	ENJON_CLASS( )
	class ArchetypeAssetLoader : public AssetLoader
	{
		ENJON_CLASS_BODY( ArchetypeAssetLoader ) 

		public:

			/**
			* @brief
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
