#ifndef ENJON_H
#define ENJON_H

/////////////////////
//  ENJON INCLUDES //
/////////////////////


#include "System/Config.h"
#include "Utils/Timing.h"
#include "Graphics/Camera2D.h"
#include "Graphics/SpriteSheet.h"
#include "Utils/FileUtils.h"
#include "Utils/Errors.h"
#include "Graphics/GLSLProgram.h"
#include "Graphics/GLTexture.h"
#include "IO/ImageLoader.h"
#include "IO/InputManager.h"
#include "IO/IOManager.h"
#include "Graphics/picoPNG.h"
#include "IO/ResourceManager.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteBatch.h"
#include "IO/TextureCache.h"
#include "Graphics/Vertex.h"
#include "Graphics/Window.h"
#include "System/Types.h"
#include "Graphics/ShaderManager.h"
#include "Defines.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon{
	extern int Init();
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif