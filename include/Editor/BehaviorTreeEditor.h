#ifndef ENJON_BEHAVIOR_TREE_EDITOR_H
#define ENJON_BEHAVIOR_TREE_EDITOR_H

/*-- Enjon includes --*/
#include "IO/InputManager.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIElement.h"
#include "Graphics/Camera2D.h"
#include "System/Internals.h"

/*-- Standard Library includes --*/
#include <unordered_map>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

namespace Enjon { namespace BehaviorTreeEditor {

	bool Init(EI::InputManager* Input, float ScreenWidth, float ScreenHeight);

	bool Update();		

	bool Draw();
}}


#endif