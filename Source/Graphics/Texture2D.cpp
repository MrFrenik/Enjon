#include "Graphics/Texture2D.h"

namespace Enjon
{
	//--------------------------------------------
	Texture2D::Texture2D()
		: mID(0), mWidth(0), mHeight(0)
	{
	}

	//--------------------------------------------
	Texture2D::~Texture2D()
	{
	}

	//--------------------------------------------
	void Texture2D::SetID(GLuint id)
	{
		mID = id;
	}

	//--------------------------------------------
	void Texture2D::SetWidth(u32& width)
	{
		mWidth = width;
	}

	//--------------------------------------------
	void Texture2D::SetHeight(u32& height)
	{
		mHeight = height;
	}
}
