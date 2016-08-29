/* ===========================================================
* @Author: 	       		John
* @Title:	       		game.h 
* @Date:   	       		2015-11-08 11:31:34
* @Last Modified time: 	2015-11-20 11:21:47
=============================================================*/

#ifndef ENJON_GAME_H
#define ENJON_GAME_H

#include <stdlib.h>
#include "Utils/Errors.h"
#include "IO/InputManager.h"
#include "Graphics/Camera2D.h"
#include "Graphics/Window.h"
#include "Graphics/ShaderManager.h"
#include "Enjon.h"

namespace Game
{
	typedef struct MainGame* Game;

	/* Create and return new game */
	extern Game NewGame(const char* Title, int ScreenWidth, int ScreenHeight);

	/* Get input manager from game */	
	extern inline Enjon::Input::InputManager* GetInput(Game game);

	/* Get Camera2D from game */	
	extern inline Enjon::Graphics::Camera2D* GetCamera(Game game);

	/* Get Window from game */	
	extern inline Enjon::Graphics::Window* GetWindow(Game game);

	/* Free game and all subsystems */
	extern inline void Run(Game game);

	/* Free game and all subsystems */
	extern inline void End(Game game);
}



#endif