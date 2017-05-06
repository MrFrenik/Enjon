// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Texture.cpp

#include "Graphics/Texture.h"
#include "Asset/TextureAssetLoader.h"

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
}

