// Copyright 2016-2017 John Jackson. All Rights Reserved.

// File: Texture.cpp

#include "Graphics/Texture.h"
#include "Asset/TextureAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Serialize/ObjectArchiver.h"
#include "Utils/FileUtils.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include <STB/stb_image_write.h> 

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#include <random>
#include <GLEW/glew.h>
#include <vector> 

namespace Enjon
{
	//=================================================

	void Texture::ExplicitConstructor( )
	{ 
		mId = 0; 
		mWidth = 0;
		mHeight = 0;
	}

	//=================================================
			
	Texture::Texture( u32 width, u32 height, u32 textureID )
		: mWidth( width ), mHeight( height ), mId( textureID )
	{ 
	} 

	//=================================================

	Texture* Texture::Construct( const String& filePath )
	{
		// Get file extension of file
		Enjon::String fileExtension = Utils::SplitString( filePath, "." ).back( ); 

		// Fields to load and store
		s32 width, height, nComps, len; 

		void* textureData = nullptr; 

		// Construct new texture to fill out
		Texture* tex = new Texture( );

		// Load HDR format
		if ( fileExtension.compare( "hdr" ) == 0 )
		{
			stbi_set_flip_vertically_on_load( true );
			f32* data = stbi_loadf( filePath.c_str( ), &width, &height, &nComps, 0 ); 

			// Store texture data
			textureData = (f32*)data;

			s32 MAG_PARAM = GL_LINEAR;
			s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
			b8 genMips = true;

			// Generate and bind texture for data storage
			glGenTextures( 1, &tex->mId );
			glBindTexture( GL_TEXTURE_2D, tex->mId );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, data ); 

			// Anisotropic filtering
			float aniso = 0.0f;
			glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

			if ( genMips )
			{
				glGenerateMipmap( GL_TEXTURE_2D );
			} 

			tex->mFormat = TextureFormat::HDR; 
		}

		// Otherwise load standard format
		else
		{
			// Load texture data
			stbi_set_flip_vertically_on_load( false );

			// For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param
			// Could optimize this later
			u8* data = stbi_load( filePath.c_str( ), &width, &height, &nComps, STBI_rgb_alpha );

			// Store texture data
			textureData = (u8*)data;

			// TODO(): For some reason, required components is not working, so just default to 4 for now
			nComps = 4;

			// Generate texture
			glGenTextures( 1, &( tex->mId ) );

			// Bind and create texture
			glBindTexture( GL_TEXTURE_2D, tex->mId );

			// Generate texture depending on number of components in texture data
			switch ( nComps )
			{
				case 3: 
				{
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data ); 
				} break;

				default:
				case 4: 
				{
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data ); 
				} break;
			}

			s32 MAG_PARAM = GL_LINEAR;
			s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
			b8 genMips = true;

			// Anisotropic filtering
			float aniso = 0.0f; 
			glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );


			if ( genMips )
			{
				glGenerateMipmap( GL_TEXTURE_2D );
			}

			tex->mFormat = TextureFormat::LDR;

			glBindTexture( GL_TEXTURE_2D, 0 ); 
		} 

		// Set texture attributes
		tex->mWidth = width;
		tex->mHeight = height;
		tex->mNumberOfComponents = nComps; 

		// Store file extension type of texture
		tex->mFileExtension = Texture::GetFileExtensionType( fileExtension ); 

		// Store texture data
		switch ( tex->mFormat )
		{
			case TextureFormat::HDR:
			{
				tex->mSourceData = new TextureSourceData< f32 >( (f32*)textureData, tex ); 
			} break;
			case TextureFormat::LDR: 
			{
				tex->mSourceData = new TextureSourceData< u32 >( (u32*)textureData, tex ); 
			} break;
		}

		return tex; 
	}

	//=================================================

	TextureFileExtension Texture::GetFileExtensionType( const Enjon::String& fileExtension )
	{
		if ( fileExtension.compare( "png" ) == 0 )
		{
			return TextureFileExtension::PNG;
		}
		else if ( fileExtension.compare( "tga" ) == 0 )
		{
			return TextureFileExtension::TGA;
		}
		else if ( fileExtension.compare( "bmp" ) == 0 )
		{
			return TextureFileExtension::BMP;
		}
		else if ( fileExtension.compare( "jpeg" ) == 0 )
		{
			return TextureFileExtension::JPEG;
		}
		else if ( fileExtension.compare( "hdr" ) == 0 )
		{
			return TextureFileExtension::HDR;
		}
		else
		{
			return TextureFileExtension::UNKNOWN;
		}
	}

	//=================================================

	TextureFormat Texture::GetFormat( ) const
	{
		return mFormat;
	}

	//=================================================

	u32 Texture::GetWidth() const
	{
		return mWidth;
	}

	//=================================================

	u32 Texture::GetHeight() const
	{
		return mHeight;
	}

	//=================================================

	u32 Texture::GetTextureId() const
	{
		return mId;
	} 

	//================================================= 

	template void Texture::WriteTextureData< f32 >( ByteBuffer* buffer ) const;
	template void Texture::WriteTextureData< u8 >( ByteBuffer* buffer ) const;

	template <typename T>
	void Texture::WriteTextureData( ByteBuffer* buffer ) const
	{
		// Get raw data from source
		const T* rawData = mSourceData->Cast< T >( )->GetData( );

		// The pixel data consists of *y scanlines of *x pixels,
		//	with each pixel consisting of N interleaved 8-bit components with no padding in between; the first
		//	pixel pointed to is top-left-most in the image.  
		u32 totalWidth = mWidth * mNumberOfComponents;

		for ( u32 h = 0; h < mHeight; ++h )
		{
			for ( u32 w = 0; w < totalWidth; ++w )
			{
				// Get index of indvidual interleaved pixel
				u32 pixelIndex = totalWidth * h + w; 
				// Raw pixel 
				T pixel = rawData[ pixelIndex ]; 
				// Write individual pixel to archive
				buffer->Write< T >( pixel );
			}
		} 
	}

	Result Texture::SerializeData( ByteBuffer* buffer ) const 
	{
		std::cout << "Serializing texture...\n";

		// Keep around texture data resource object for serialization purposes
		// Will be compiled out with release of application and only defined as being with editor data

		// Write out basic header info for texture 
		buffer->Write< u32 >( mWidth );					// Texture width
		buffer->Write< u32 >( mHeight );				// Texture height
		buffer->Write< u32 >( mNumberOfComponents );	// Texture components per pixel
		buffer->Write< u32 >( ( u32 )mFormat );			// Texture format
		buffer->Write< u32 >( ( u32 )mFileExtension );	// Texture file extension

		switch ( mFormat )
		{
			case TextureFormat::HDR:
			{
				// Write texture data
				WriteTextureData< f32 >( buffer );

				// Release source data after serializing
				const_cast< TextureSourceData< f32 >* >( mSourceData->Cast< f32 >( ) )->ReleaseData( );
			} break;

			case TextureFormat::LDR:
			{
				// Write texture data
				WriteTextureData< u8 >( buffer );

				// Release source data after serializing
				const_cast< TextureSourceData< u8 >* >( mSourceData->Cast< u8 >( ) )->ReleaseData( ); 
			} break;
		} 

		return Result::SUCCESS;
	} 
	
	Result Texture::DeserializeData( ByteBuffer* buffer )
	{
		std::cout << "Deserializing texture...\n";

		// Read properties from buffer - THIS SHOULD BE USED WITH A VERSIONING STRUCT!
		mWidth				= buffer->Read< u32 >( );							// Texture width
		mHeight				= buffer->Read< u32 >( );							// Texture height
		mNumberOfComponents = buffer->Read< u32 >( );							// Texture components per pixel
		mFormat				= TextureFormat( buffer->Read< u32 >( ) );			// Texture format
		mFileExtension		= TextureFileExtension( buffer->Read< u32 >( ) );	// Texture format

		switch ( mFormat )
		{
			case TextureFormat::HDR:
			{ 
				// Total width of pixel array
				u32 totalWidth = mWidth * mNumberOfComponents;
				// Buffer for pixel data
				f32* pixelData = new f32[ mHeight * totalWidth]; 

				// Fill out pixel data
				for ( u32 h = 0; h < mHeight; ++h ) 
				{
					for ( u32 w = 0; w < totalWidth; ++w )
					{
						// Get index of indvidual interleaved pixel
						u32 pixelIndex = totalWidth * h + w; 
						// Read individual pixel from archive
						pixelData[ pixelIndex ] = buffer->Read< f32 >( );
					}
				} 

				// Generate texture
				glGenTextures( 1, &( mId ) ); 
				// Bind texture to be created
				glBindTexture( GL_TEXTURE_2D, mId );

				switch ( mNumberOfComponents )
				{
					default:
					case 3: 
					{
						glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, pixelData );
					} break;

					case 4: 
					{
						glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, pixelData );
					} break;
				}

				s32 MAG_PARAM = GL_LINEAR;
				s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
				b8 genMips = true;

				// Anisotropic filtering
				float aniso = 0.0f;
				glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
				aniso = std::min( aniso, 4.0f );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

				if ( genMips )
				{
					glGenerateMipmap( GL_TEXTURE_2D );
				} 

				glBindTexture( GL_TEXTURE_2D, 0 );

				// Clean up pixel data once done
				delete[] pixelData;
				pixelData = nullptr;
			} break;

			case TextureFormat::LDR:
			{ 
				// Buffer for pixel data
				u8* pixelData = new u8[ mHeight * mWidth * mNumberOfComponents ];

				u32 totalWidth = mWidth * mNumberOfComponents;

				for ( u32 h = 0; h < mHeight; ++h ) 
				{
					for ( u32 w = 0; w < totalWidth; ++w )
					{
						// Get index of indvidual interleaved pixel
						u32 pixelIndex = totalWidth * h + w; 
						// Read individual pixel from archive
						pixelData[ pixelIndex ] = buffer->Read< u8 >( );
					}
				} 

				// Generate texture
				glGenTextures( 1, &( mId ) ); 
				// Bind texture to be created
				glBindTexture( GL_TEXTURE_2D, mId );

				switch ( mNumberOfComponents )
				{
					default:
					case 3: 
					{
						glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData ); 
					} break;

					case 4: 
					{
						glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData ); 
					} break;
				}

				s32 MAG_PARAM = GL_LINEAR;
				s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
				b8 genMips = true;

				// Anisotropic filtering
				float aniso = 0.0f;
				glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

				if ( genMips )
				{
					glGenerateMipmap( GL_TEXTURE_2D );
				} 

				glBindTexture( GL_TEXTURE_2D, 0 ); 

				// Clean up pixel data once done
				delete[] pixelData;
				pixelData = nullptr; 

			} break;
		} 

		return Result::SUCCESS;
	}
}














