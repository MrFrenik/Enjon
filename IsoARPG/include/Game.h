#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include "Application.h"

namespace Enjon
{
	class Entity;
	class EntityManager;

	namespace Graphics
	{
		class QuadBatch;
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
		Enjon::Entity* mGun 					= nullptr;
		Enjon::Entity* mGreen 					= nullptr;
		Enjon::Entity* mRed 					= nullptr;
		Enjon::Graphics::DirectionalLight* mSun = nullptr;
		Enjon::Graphics::QuadBatch* mBatch 		= nullptr;

		bool mShowEntities = true;
};

#endif