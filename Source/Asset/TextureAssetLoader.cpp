// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: TextureAssetLoader.cpp

#include "Asset/AssetManager.h"
#include "Asset/TextureAssetLoader.h" 
#include "IO/IOManager.h"
#include "Utils/FileUtils.h"
#include "Math/Vec3.h"
#include "Engine.h"

#include <random>
#include <GLEW/glew.h>
#include <vector> 

namespace Enjon
{
	TextureAssetLoader::TextureAssetLoader()
	{ 
	}
			
	TextureAssetLoader::~TextureAssetLoader()
	{ 
	} 

	//============================================================================================== 

	Asset* TextureAssetLoader::LoadResourceFromFile(const String& filePath, const String& name )
	{ 
		// Load and construct texture from file
		Enjon::Texture* tex = Texture::Construct( filePath );

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

	Result TextureAssetLoader::Cache( ByteBuffer& buffer, Asset* asset )
	{
		// Call super
		AssetLoader::Cache( buffer, asset ); 

		// Return result
		return Result::SUCCESS;
	}
}
