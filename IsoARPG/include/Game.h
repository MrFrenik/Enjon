#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include "Application.h"

#include <vector>

namespace Enjon
{
	class Entity;
	class EntityManager; 
	class QuadBatch;
	class DirectionalLight;
	class Material;
	class Mesh;
}

class Game : public Enjon::Application
{
	public:
		Game();
		~Game();

		virtual void Initialize();  
		virtual void Update(f32 dt);
		virtual void Shutdown();

		void ProcessInput(f32 dt);

	private:
		void ListEntityChildren(Enjon::Entity* entity, u32 indentAmount);

	private:
		Enjon::EntityManager* mEntities 		= nullptr;
		Enjon::Entity* mGun 					= nullptr;
		Enjon::Entity* mGreen 					= nullptr;
		Enjon::Entity* mRed 					= nullptr;
		Enjon::DirectionalLight* mSun = nullptr;
		Enjon::QuadBatch* mBatch 		= nullptr;

		Enjon::Material* mGunMat 	= nullptr;
		Enjon::Material* mRedMat 	= nullptr;
		Enjon::Material* mGreenMat 	= nullptr;
		Enjon::Material* mBlueMat 	= nullptr;
		Enjon::Mesh* mSphereMesh 	= nullptr;
		Enjon::Mesh* mGunMesh 		= nullptr;
		Enjon::Mesh* mBuddhaMesh 	= nullptr;

		std::vector<Enjon::Entity*> mHandles;

		bool mShowEntities = true;
		bool mMovementOn = true;

		f32 mCameraSpeed = 0.12f;
};

#endif