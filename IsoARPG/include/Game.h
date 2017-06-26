#ifndef ARPG_GAME_H
#define ARPG_GAME_H

#include <Application.h> 
#include <Asset/Asset.h>
#include <Entity/EntityManager.h>
#include <Graphics/ShaderGraph.h>
#include <Graphics/Shader.h>

#include <vector>

namespace Enjon
{
	class Entity;
	class EntityManager; 
	class QuadBatch;
	class DirectionalLight;
	class Material;
	class Mesh;
	class DeferredRenderer;
	class Input;
	class UIFont;
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
		Enjon::DeferredRenderer* mGfx		= nullptr;
		Enjon::Input* mInput				= nullptr;
		Enjon::EntityManager* mEntities		= nullptr;

		Enjon::EntityHandle mGun;
		Enjon::EntityHandle mGreen;
		Enjon::EntityHandle mRed;
		Enjon::EntityHandle mBlue;
		Enjon::EntityHandle mRock;
		Enjon::EntityHandle mRock2;

		Enjon::DirectionalLight* mSun	= nullptr;
		Enjon::QuadBatch* mBatch 		= nullptr;
		Enjon::QuadBatch* mTextBatch 	= nullptr;

		Enjon::Material* mGunMat 	= nullptr;
		Enjon::Material* mRedMat 	= nullptr;
		Enjon::Material* mGreenMat 	= nullptr;
		Enjon::Material* mBlueMat 	= nullptr;
		Enjon::Material* mFloorMat	= nullptr;
		Enjon::Material* mFontMat	= nullptr;
		Enjon::Material* mRockMat	= nullptr;
		Enjon::Material* mPlasticMat	= nullptr;
		Enjon::Material* mGoldMat	= nullptr;

		Enjon::AssetHandle< Enjon::Mesh > mSphereMesh;
		Enjon::AssetHandle< Enjon::Mesh > mGunMesh;
		Enjon::AssetHandle< Enjon::Mesh > mBuddhaMesh;
		Enjon::AssetHandle< Enjon::Mesh > mMonkeyMesh; 
		Enjon::AssetHandle< Enjon::UIFont > mFont;
		
		std::vector<Enjon::AssetHandle<Enjon::Mesh>*> mMeshSwitches; 

		std::vector<Enjon::Entity*> mHandles; 

		bool mShowEntities = true;
		bool mMovementOn = false;

		f32 mCameraSpeed = 0.12f;

		f32 mFontSize = 1.0f;

		Enjon::String mWorldString = ""; 

		Enjon::Texture* mTex = nullptr;
};

#endif