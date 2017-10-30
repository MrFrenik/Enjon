// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Texture.cpp

#include "Graphics/Texture.h"
#include "Asset/TextureAssetLoader.h"
#include "Asset/AssetManager.h"
#include "Engine.h"

namespace Enjon
{
	//=================================================

	Texture::Texture()
		: mId(0), mWidth(0), mHeight(0)
	{ 

	}

	//=================================================
			
	Texture::Texture( u32 width, u32 height, u32 textureID )
		: mWidth( width ), mHeight( height ), mId( textureID )
	{ 
	}

	//=================================================

	Texture::~Texture()
	{ 

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
			
	Enjon::Result Texture::CacheFile( Enjon::ByteBuffer& buffer )
	{
		// Get engine
		Enjon::Engine* engine = Enjon::Engine::GetInstance( );

		// Get asset manager from subsystem catalog
		const Enjon::AssetManager* am = engine->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

		// Get project directory
		const Enjon::String& cachePath =  am->GetCachedAssetsPath( );

		// Write out to buffer
		buffer.Write( mWidth );
		buffer.Write( mHeight );
		buffer.Write( (u32)mFileExtension );

		return Enjon::Result::SUCCESS;
	}
}

