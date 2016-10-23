#ifndef ENJON_H
#define ENJON_H

/////////////////////
//  ENJON INCLUDES //
/////////////////////


#include "System/Config.h"
#include "System/Types.h"
#include "System/Containers.h"

#include "Utils/Timing.h"
#include "Utils/FileUtils.h"
#include "Utils/Errors.h"
#include "Utils/json.h"

#include "Math/Maths.h"
#include "Math/Random.h"

#include "AI/PathFinding.h"
#include "AI/SpatialHash.h"

#include "Graphics/Window.h"
#include "Graphics/Camera2D.h"
#include "Graphics/SpriteSheet.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/GLSLProgram.h"
#include "Graphics/GLTexture.h"
#include "Graphics/picoPNG.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/Vertex.h"
#include "Graphics/Window.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/SpriteSheetManager.h"
#include "Graphics/Font.h"
#include "Graphics/FontManager.h"
#include "Graphics/ParticleEngine2D.h"
#include "Graphics/FrameBufferObject.h"
#include "Graphics/DeferredRenderer.h"
#include "Graphics/GBuffer.h"
#include "Graphics/Animations.h"
#include "Graphics/Shapes.h"

#include "GUI/Signal.h"
#include "GUI/Property.h"
#include "GUI/GUIElement.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIAnimationElement.h"

#include "IO/ImageLoader.h"
#include "IO/InputManager.h"
#include "IO/IOManager.h"
#include "IO/ResourceManager.h"
#include "IO/TextureCache.h"
#include "IO/ObjLoader.h"

#include "Physics/AABB.h"

#include "BehaviorTree/BT.h"

#include "Defines.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon {
	
	inline int Init()
	{
		 //Initialize SDL
		SDL_Init(SDL_INIT_EVERYTHING);
		
		//Tell SDL that we want a double buffered window so we dont get any flickering
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,      8);
		 
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,        32);
		 
		SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);
		 
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  16);

		return 0;
	}
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif