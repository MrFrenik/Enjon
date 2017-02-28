#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include "Application.h"

class Game : public Enjon::Application
{
	public:
		Game();
		~Game();

		virtual void Initialize();  
		virtual void Update(Enjon::f32 dt);
		virtual void Shutdown();

	private:
};

#endif