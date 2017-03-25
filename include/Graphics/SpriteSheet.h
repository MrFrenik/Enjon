#ifndef ENJON_SPRITESHEET_H
#define ENJON_SPRITESHEET_H

#include "Graphics/GLTexture.h"
#include "Math/Maths.h"

namespace Enjon { 

	struct SpriteSheet
	{
		SpriteSheet()
		{}

		void Init(const GLTexture& Texture, const Enjon::iVec2 Dims)
		{
			this->texture = Texture;
			this->dims = Dims;
			this->init_status = 1;
		}

		inline bool IsInit() { return this->init_status; }

		Vec4 GetUV(int index)
		{
			int xTile = index % dims.x;
			int yTile = index / dims.x;
			
			Vec4 uv;
			uv.x = xTile / (float)dims.x;
			uv.y = yTile / (float)dims.y;
			uv.z = 1.0f / dims.x;
			uv.w = 1.0f / dims.y;

			return uv; 
		}

	GLTexture texture;
	Enjon::iVec2 dims;
	int init_status = 0;

	}; 
}


#endif