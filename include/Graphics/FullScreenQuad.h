#ifndef ENJON_FULL_SCREEN_QUAD
#define ENJON_FULL_SCREEN_QUAD

#include "System/Types.h"
#include "GLEW/glew.h"

namespace Enjon {

	class FullScreenQuad
	{
		public:
			FullScreenQuad();
			~FullScreenQuad();

			void Bind();
			void Unbind();
			void Submit();

		private:
			GLuint		mVAO; 		// VAO ID
			GLuint		mVBO; 		// VBO id
	};

}

#endif