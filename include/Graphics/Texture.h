// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Texture.h

#pragma once
#ifndef ENJON_TEXTURE_H
#define ENJON_TEXTURE_H

#include "System/Types.h"
#include "Asset/Asset.h" 

namespace Enjon
{
	class TextureAssetLoader;

	enum class TextureFileExtension : u32
	{
		PNG,
		TGA,
		JPEG,
		BMP,
		HDR,
		UNKNOWN
	};

	class Texture : public Asset
	{
		friend TextureAssetLoader;

		ENJON_OBJECT( Texture )

		public:	

			/**
			* @brief Constructor
			*/
			Texture();
			
			/**
			* @brief Constructor
			*/
			Texture( u32 width, u32 height, u32 textureID );

			/**
			* @brief Destructor
			*/
			~Texture();

			/**
			* @brief 
			*/
			u32 GetWidth() const;

			/**
			* @brief
			*/
			u32 GetHeight() const;

			/**
			* @brief
			*/
			u32 GetTextureId() const;

		protected: 
			/*
			* @brief
			*/
			static TextureFileExtension GetFileExtensionType( const Enjon::String& fileExtension );

		protected:
			/*
			* @brief
			*/
			virtual Enjon::Result CacheFile( Enjon::ByteBuffer& buffer ) override; 

		private:
			u32 mId;
			u32 mWidth;
			u32 mHeight;
			TextureFileExtension mFileExtension;
	}; 
}

#endif
