#ifndef ENJON_TEXTURE_2D
#define ENJON_TEXTURE_2D

#include "System/Types.h"
#include "Resource/Resource.h"

#include <GLEW/glew.h>

namespace Enjon
{
	class Texture2D : public Resource
	{
		public:
			Texture2D();
			~Texture2D();

			u32 GetWidth() const { return mWidth; }
			u32 GetHeight() const { return mHeight; }

		private:
			void SetID(GLuint id);
			void SetWidth(u32& width);
			void SetHeight(u32& height);

			GLuint mID;
			u32 mWidth;
			u32 mHeight;
	};	
}

#endif