// /* ===========================================================
// * @Author: 	       		John
// * @Title:	       		game.cpp 
// * @Date:   	       		2015-11-08 11:31:34
// * @Last Modified time: 	2016-02-11 08:00:46
// =============================================================*/

// #include <Utils/Errors.h>
// #include "Game.h"

// namespace Game
// {
// 	struct MainGame
// 	{
// 		Enjon::Input::InputManager* input;	
// 		Enjon::Graphics::Camera2D* camera;	
// 		Enjon::Graphics::Window* window;	
// 	};

// 	struct MainGame* NewGame(const char* Title, int ScreenWidth, int ScreenHeight)
// 	{
// 		struct MainGame* game = (struct MainGame*)malloc(sizeof *game);

// 		if (!game)
// 		{
// 			Enjon::Utils::FatalError("GAME::NEWGAME::Could not allocate memory for game.\n");
// 		}

// 		// Init Enjon
// 		Enjon::Init();

// 		// Create systems	
// 		game->input = new Enjon::Input::InputManager;
// 		game->camera = new Enjon::Graphics::Camera2D;
// 		game->window = new Enjon::Graphics::Window;

// 		// Init Window
// 		game->window->Init(Title, ScreenWidth, ScreenHeight);

// 		// Init camera
// 		game->camera->Init(ScreenWidth, ScreenHeight);

// 		// Init shader manager
// 		Enjon::Graphics::ShaderManager::Init();

// 		return game; 
// 	}

// 	 Get input manager from game 	
// 	inline Enjon::Input::InputManager* GetInput(struct MainGame* game)
// 	{
// 		if (!game) Enjon::Utils::FatalError("GAME::GET_INPUT::Cannot operate on NULL data.\n");
// 		return game->input;
// 	}

// 	/* Get Camera2D from game */	
// 	inline Enjon::Graphics::Camera2D* GetCamera(struct MainGame* game)
// 	{
// 		if (!game) Enjon::Utils::FatalError("GAME::GET_CAMERA::Cannot operate on NULL data.\n");
// 		return game->camera;
// 	}


// 	inline Enjon::Graphics::Window* GetWindow(struct MainGame* game)
// 	{
// 		if (!game) Enjon::Utils::FatalError("GAME::GET_WINDOW::Cannot operate on NULL data.\n");
// 		return game->window;
// 	}

// 	/* Run game loop */
// 	inline void Run(Game game)
// 	{
// 		// NOTE(John): Nothing for now...
// 	}

// 	/* Free game and all subsystems */
// 	inline void End(Game game)
// 	{
// 		if (!game) Enjon::Utils::FatalError("GAME::END::Cannot operate on NULL data.\n");

// 		// Free subsystems
// 		Enjon::Graphics::Camera2D* camera = game->camera;
// 		Enjon::Graphics::Window* window = game->window;
// 		Enjon::Input::InputManager* input = game->input;

// 		if (camera) delete(camera);	
// 		if (window) delete(window);	
// 		if (input) delete(input);

// 		delete(game);	
// 	}

// }















