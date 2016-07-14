#ifndef ENJON_H
#define ENJON_H

/////////////////////
//  ENJON INCLUDES //
/////////////////////


#include "System/Config.h"
#include "System/Types.h"

#include "Utils/Timing.h"
#include "Utils/FileUtils.h"
#include "Utils/Errors.h"

#include "Math/Maths.h"
#include "Math/Random.h"

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

		return 0;
	}
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif