#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include "Application.h" 
#include "Asset/Asset.h"

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

		virtual Enjon::Result Initialize();  
		virtual Enjon::Result Update(f32 dt);
		virtual Enjon::Result Shutdown();

		virtual Enjon::Result ProcessInput(f32 dt);

	private:
		void ListEntityChildren(Enjon::Entity* entity, u32 indentAmount);

	private:
		Enjon::EntityManager* mEntities = nullptr;
		Enjon::Entity* mGun 			= nullptr;
		Enjon::Entity* mGreen 			= nullptr;
		Enjon::Entity* mRed 			= nullptr; 

		Enjon::DirectionalLight* mSun	= nullptr;
		Enjon::QuadBatch* mBatch 		= nullptr;

		Enjon::Material* mGunMat 	= nullptr;
		Enjon::Material* mRedMat 	= nullptr;
		Enjon::Material* mGreenMat 	= nullptr;
		Enjon::Material* mBlueMat 	= nullptr;
		Enjon::Material* mFloorMat	= nullptr;

		Enjon::AssetHandle<Enjon::Mesh> mSphereMesh;
		Enjon::AssetHandle<Enjon::Mesh> mGunMesh;
		Enjon::AssetHandle<Enjon::Mesh> mBuddhaMesh;
		Enjon::AssetHandle<Enjon::Mesh> mMonkeyMesh;

		std::vector<Enjon::AssetHandle<Enjon::Mesh>*> mMeshSwitches;

		std::vector<Enjon::Entity*> mHandles;

		bool mShowEntities = true;
		bool mMovementOn = true;

		f32 mCameraSpeed = 0.12f;
};

#endif