// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/AssetManager.h"
#include "Asset/TextureAssetLoader.h" 
#include "Graphics/picoPNG.h"
#include "IO/IOManager.h"
#include "Utils/FileUtils.h"
#include "Engine.h"

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

			stbi_image_free( data ); 
		}

		// Otherwise load standard format
		else
		{
			// Load texture data
			stbi_set_flip_vertically_on_load( false );
			u8* data = stbi_load( filePath.c_str( ), &width, &height, &nComps, STBI_rgb_alpha );

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
		
			// No longer need data, so free
			stbi_image_free( data );
		} 

		// Set texture attributes
		tex->mWidth = width;
		tex->mHeight = height;

		// Generate new UUID
		tex->mUUID = Enjon::UUID::GenerateUUID( );

#if CACHING 
		//Cache image data to file
		auto saveData = stbi_write_png_to_mem( data, 0, width, height, 4, &len );
		CacheTextureData( saveData, len, tex );
		stbi_image_free( saveData );
#endif 

		// Store file extension type of texture
		tex->mFileExtension = Texture::GetFileExtensionType( fileExtension ); 

		return tex;
	}
			
	//============================================================================================== 

	Texture* TextureAssetLoader::LoadAssetFromFile(const String& filePath, const String& name )
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
		// Get assets directory
		Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
		Enjon::String assetDir = am->GetAssetsPath( );

		// Get default texture
		Enjon::Texture* tex = LoadTextureFromFile( assetDir + "/Textures/white.png" ); 
		tex->mName = AssetLoader::GetQualifiedName( assetDir + "/Textures/white.png" );

		// Set default texture
		mDefaultAsset = tex;
	} 
	
	Result TextureAssetLoader::CacheTextureData( const u8* data, u32 length, Texture* texture )
	{
		// Get engine
		Enjon::Engine* engine = Enjon::Engine::GetInstance( );

		// Get asset manager from subsystem catalog
		const Enjon::AssetManager* am = engine->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

		// Get project directory
		const Enjon::String& cachePath =  am->GetCachedAssetsPath( ); 
		
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
		writeBuffer.WriteToFile( cachePath + texture->mUUID.ToString( ) ); 

		return Enjon::Result::SUCCESS;
	}
}
