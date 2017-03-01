#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include "Application.h"

namespace Enjon
{
	class EntityHandle;
	class EntityManager;

	namespace Graphics
	{
		class DirectionalLight;
	}
}

class Game : public Enjon::Application
{
	public:
		Game();
		~Game();

		virtual void Initialize();  
		virtual void Update(Enjon::f32 dt);
		virtual void Shutdown();

	private:
		Enjon::EntityManager* mEntities 		= nullptr;
		Enjon::EntityHandle* mHandle 			= nullptr;
		Enjon::Graphics::DirectionalLight* mSun = nullptr;
};

#endif