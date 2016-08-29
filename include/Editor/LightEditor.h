#ifndef ENJON_LIGHT_EDITOR_H
#define ENJON_LIGHT_EDITOR_H

/*-- Enjon includes --*/
#include "Graphics/FrameBufferObject.h"
#include "IO/InputManager.h"

/*-- Standard Library includes --*/
#include <unordered_map>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

namespace Enjon { namespace LightEditor {

	bool Init(EI::InputManager* _Input, EG::Window* _Window, float SW, float SH, EG::FrameBufferObject* diffuse_FBO, EG::FrameBufferObject* normals_FBO, EG::FrameBufferObject* depth_FBO, EG::FrameBufferObject* deferred_FBO);

	bool Update();		

	bool Draw();
}}


#endif