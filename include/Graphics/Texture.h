// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Texture.h

#pragma once
#ifndef ENJON_TEXTURE_H
#define ENJON_TEXTURE_H

#include "System/Types.h"
#include "Asset/Asset.h" 

namespace Enjon
{
	// Forward declarations
	template <typename T>
	class TextureSourceData;
	class TextureSourceDataBase;
	class TextureAssetLoader;
	class Texture;

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

	ENJON_ENUM( )
	enum class TextureFormat : u32
	{
		LDR,
		HDR
	};

	ENJON_ENUM( )
	enum class TextureMipMapFormat : u32
	{
		None,
		Nearest,
		Linear
	}; 

	ENJON_ENUM( )
	enum class TextureWrapMode : u32
	{
		Repeat,
		ClampToEdge
	};

	class TextureSourceDataBase
	{
		friend TextureAssetLoader;
		friend Texture;
		public:

			/*
			* @brief
			*/
			TextureSourceDataBase( ) = default;

			/*
			* @brief
			*/
			~TextureSourceDataBase( ) = default;

			template <typename T>
			const TextureSourceData<T>* Cast( )
			{
				return static_cast< TextureSourceData<T>* >( this );
			}

		protected: 
			virtual void Base( ) = 0;

		protected:
			const Texture* mOwner = nullptr;
	};

	template <typename T>
	class TextureSourceData : public TextureSourceDataBase
	{
		friend Texture;
		friend TextureAssetLoader;
		public:
			TextureSourceData( ) = default;

			TextureSourceData( T* data, const Texture* owner )
				: mData( data )
			{ 
				mOwner = owner;
			}

			~TextureSourceData( )
			{
				ReleaseData( );
			}
		
		protected:
			virtual void Base( )
			{ 
			}

			const T* GetData( ) const
			{
				return mData;
			}

		protected:
			void ReleaseData( )
			{
				delete mData;
				mData = nullptr;
			}

		private:
			T* mData;
	}; 

	ENJON_CLASS( Construct )
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
 
			/**
			* @brief
			*/
			ENJON_FUNCTION( )
			TextureFormat GetFormat( ) const;

		protected: 
			/*
			* @brief
			*/
			static TextureFileExtension GetFileExtensionType( const Enjon::String& fileExtension );

		public:
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* archiver ) const override;
			
			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* archiver ) override;

		protected: 

			/*
			* @brief
			* @note Need to eventually move this into a factory class as a part of the graphics subsystem
			*/
			static Texture* Construct( const String& filePath );

			/*
			* @brief
			*/
			template <typename T>
			void WriteTextureData( ByteBuffer* archiver ) const;

		private:
			
			ENJON_PROPERTY( )
			u32 mId;

			ENJON_PROPERTY( )
			u32 mWidth;

			ENJON_PROPERTY( )
			u32 mHeight; 

			ENJON_PROPERTY( )
			u32 mNumberOfComponents;

			ENJON_PROPERTY( )
			TextureFileExtension mFileExtension;

			ENJON_PROPERTY( )
			TextureFormat mFormat;

			TextureSourceDataBase* mSourceData = nullptr;
	}; 

}

#endif
