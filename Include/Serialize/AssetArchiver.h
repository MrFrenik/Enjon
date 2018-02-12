// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetArchiver.h

#pragma once 
#ifndef ENJON_ASSET_ARCHIVER_H
#define ENJON_ASSET_ARCHIVER_H 

#include "Base/Object.h" 
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"

namespace Enjon
{
	class Asset; 

	class AssetArchiver : public ObjectArchiver
	{
		public: 

			/*
			* @brief
			*/
			AssetArchiver( ) = default;

			/*
			* @brief
			*/
			~AssetArchiver( ) = default;

			/*
			* @brief
			*/
			Result Serialize( const Asset* asset ); 

			/*
			* @brief
			*/
			Result Deserialize( const String& filePath, Vector< Asset* >& out );

			/*
			* @brief
			*/
			Asset* Deserialize( const String& filePath );

			/*
			* @brief
			*/
			static void Deserialize( ByteBuffer* buffer, Asset* asset );

		protected:

		private: 
	};

}

#endif
