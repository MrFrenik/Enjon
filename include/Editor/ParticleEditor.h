#ifndef ENJON_PARTICLE_EDITOR_H
#define ENJON_PARTICLE_EDITOR_H

/*-- Enjon includes --*/
#include "Graphics/Camera2D.h"

/*-- Standard Library includes --*/
#include <unordered_map>


namespace Enjon { namespace Editor { namespace Particle{

	bool Init(EI::InputManager* Input, float ScreenWidth, float ScreenHeight);

	bool Update();		

	bool Draw();

}}}


#endif