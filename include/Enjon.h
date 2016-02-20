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
#include "Graphics/Font.h"
#include "Graphics/FontManager.h"
#include "Graphics/ParticleEngine2D.h"

#include "IO/ImageLoader.h"
#include "IO/InputManager.h"
#include "IO/IOManager.h"
#include "IO/ResourceManager.h"
#include "IO/TextureCache.h"

#include "Physics/AABB.h"

#include "Defines.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon{
	extern int Init();
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif