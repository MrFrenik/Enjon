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

	ENJON_ENUM( )
	enum class TextureFileExtension : u32
	{
		PNG,
		TGA,
		JPEG,
		BMP,
		HDR,
		UNKNOWN
	};

	ENJON_CLASS( )
	class Texture : public Asset
	{
		friend TextureAssetLoader;

		ENJON_CLASS_BODY( )

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
			ENJON_FUNCTION( )
			Enjon::u32 GetWidth() const;

			/**
			* @brief
			*/
			ENJON_FUNCTION( )
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
			
			ENJON_PROPERTY( )
			u32 mId;

			ENJON_PROPERTY( )
			u32 mWidth;

			ENJON_PROPERTY( )
			u32 mHeight; 

			ENJON_PROPERTY( )
			TextureFileExtension mFileExtension;
	}; 

}

#endif
