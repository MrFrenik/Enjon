// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/AssetManager.h"
#include "Asset/TextureAssetLoader.h" 
#include "Graphics/picoPNG.h"
#include "IO/IOManager.h"
#include "Utils/FileUtils.h"
#include "Math/Vec3.h"
#include "Engine.h"

#include <random>
#include <GLEW/glew.h>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB/stb_image_write.h> 

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#define CACHING 0

namespace Enjon
{
	TextureAssetLoader::TextureAssetLoader()
	{ 
	}
			
	TextureAssetLoader::~TextureAssetLoader()
	{ 
	} 

	Texture* TextureAssetLoader::LoadTextureFromFile( const Enjon::String& filePath )
	{
		// Get file extension of file
		Enjon::String fileExtension = Utils::SplitString( filePath, "." ).back( ); 
		
		// Create new texture
		Enjon::Texture* tex = new Enjon::Texture;
		
		// Fields to load and store
		s32 width, height, nComps, len; 

		void* textureData = nullptr;

		// Load HDR format
		if ( fileExtension.compare( "hdr" ) == 0 )
		{
			stbi_set_flip_vertically_on_load( true );
			f32* data = stbi_loadf( filePath.c_str( ), &width, &height, &nComps, 0 ); 

			glGenTextures( 1, &tex->mId );
			glBindTexture( GL_TEXTURE_2D, tex->mId );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, data );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			s32 MAG_PARAM = GL_LINEAR;
			s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
			b8 genMips = true;

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

			if ( genMips )
			{
				glGenerateMipmap( GL_TEXTURE_2D );
			} 

			// Free image data once done
			stbi_image_free( data );
		}

		// Otherwise load standard format
		else
		{
			// Load texture data
			stbi_set_flip_vertically_on_load( false );
			u8* data = stbi_load( filePath.c_str( ), &width, &height, &nComps, STBI_rgb_alpha );
			textureData = (u8*)data;

			// Generate texture
			glGenTextures( 1, &( tex->mId ) );

			// Bind and create texture
			glBindTexture( GL_TEXTURE_2D, tex->mId );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

			s32 MAG_PARAM = GL_LINEAR;
			s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
			b8 genMips = true;

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

			if ( genMips )
			{
				glGenerateMipmap( GL_TEXTURE_2D );
			}

			glBindTexture( GL_TEXTURE_2D, 0 ); 
		} 

		// Set texture attributes
		tex->mWidth = width;
		tex->mHeight = height;

		// Generate new UUID
		tex->mUUID = Enjon::UUID::GenerateUUID( );

#if CACHING 
		//Cache image data to file
		if ( textureData != nullptr )
		{
			auto saveData = stbi_write_png_to_mem( (u8*)textureData, 0, width, height, 4, &len );
			TextureAssetLoader::CacheTextureData( saveData, len, tex );
			stbi_image_free( saveData ); 

			// Free image data
			stbi_image_free( (u8*)textureData );
		}
#else
		stbi_image_free( textureData );
#endif 
 
		// Store file extension type of texture
		tex->mFileExtension = Texture::GetFileExtensionType( fileExtension ); 

		return tex;
	}
			
	//============================================================================================== 

	Texture* TextureAssetLoader::LoadResourceFromFile(const String& filePath, const String& name )
	{ 
		// Load texture
		Enjon::Texture* tex = LoadTextureFromFile( filePath );
		tex->mName = name;

		// Add to assets with qualified name
		AddToAssets(name, tex); 

		return tex; 
	} 

	void TextureAssetLoader::RegisterDefaultAsset( )
	{ 
		u32 texID;
		u32 width = 16;
		u32 height = 16;

		const Enjon::Vec3 white( 1.0f );
		const Enjon::Vec3 blue( 0.3f, 0.5f, 0.7f );
		
		// Generate checkerboard texture
		std::vector< Enjon::Vec3 > data;
		for ( u32 row = 0; row < height; ++row )
		{
			for ( u32 col = 0; col < width; ++col ) 
			{
				if ( row == 0 || row == 1 || row == 2 || row == 3 || row == 8 || row == 9 || row == 10 || row == 11 )
				{
					if ( col == 0 || col == 1 || col == 2 || col == 3 || col == 8 || col == 9 || col == 10 || col == 11 )
					{
						data.push_back( blue );
					}
					else
					{
						data.push_back( white );
					}
				}
				else if ( row == 4 || row == 5 || row == 6 || row == 7 || 12 || row == 13 || row == 14 || row == 15 )
				{
					if ( col == 0 || col == 1 || col == 2 || col == 3 || col == 8 || col == 9 || col == 10 || col == 11 )
					{
						data.push_back( white );
					}
					else
					{
						data.push_back( blue );
					}
				}
			}
		}

		glGenTextures( 1, &texID );
		glBindTexture( GL_TEXTURE_2D, texID );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, &data[ 0 ] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); 
		glGenerateMipmap( GL_TEXTURE_2D );
		
		Enjon::Texture* defaultTex = new Enjon::Texture( width, height, texID );
		defaultTex->mName = "defaultTexture";

		// Set default texture
		mDefaultAsset = defaultTex;
	} 
	
	Result TextureAssetLoader::CacheTextureData( const u8* data, u32 length, Texture* texture )
	{
		// Get engine
		Enjon::Engine* engine = Enjon::Engine::GetInstance( );

		// Get asset manager from subsystem catalog
		Enjon::AssetManager* am = engine->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

		// Get project directory
		String outputPath = am->GetAssetsPath( ) + "/Cache/" + texture->mUUID.ToString() + ".easset";
		
		// Byte buffer to write to
		Enjon::ByteBuffer writeBuffer;
		
		// Add data to byte buffer
		writeBuffer.Write( texture->mWidth );
		writeBuffer.Write( texture->mHeight );
		writeBuffer.Write( length );
		writeBuffer.Write( ( u32 )texture->mFileExtension );
		for ( usize i = 0; i < length; ++i )
		{
			writeBuffer.Write( data[ i ] );
		} 

		// Write out to file
		writeBuffer.WriteToFile( outputPath ); 

		return Enjon::Result::SUCCESS;
	}
}
